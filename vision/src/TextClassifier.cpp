#include "../include/TextClassifier.h"

/**
 * @param context The zeroMQ context with which to creates with
 * @param textClassifierEndpoint Endpoint of the text classifier
 * @param pythonServerEndpoint Endpont of the python server
 */
TextClassifier::TextClassifier(zmqpp::context& context,
                               const std::string& textClassifierEndpoint)
    : IModel("text_classifer.txt", context) {
  try {
    this->replySocket.bind(textClassifierEndpoint);
  } catch (const zmqpp::exception& e) {
    LOG(FATAL) << e.what();
  }
}

/**
 * Handle classification via text extraction.
 *
 * @param imagePath Path to the image to extract text from
 * @return pair <item identification success/fail, string>
 */
OCRResult TextClassifier::handleClassification(const std::filesystem::path& imagePath) {
  this->logger.log("Entering handleClassification, running model");
  OCRResult result = this->runModel(imagePath);
  this->logger.log("Output handled. Returning OCRResult to model handler.");
  return result;
}

/**
 * Handle calling python script to run text extraction.
 *
 * Input:
 * @param imagePath path to the image to extract text from
 * @return The result send back from the python server
 */
OCRResult TextClassifier::runModel(const std::filesystem::path& imagePath) {
  this->logger.log("Entering runModel");
  OCRResult classifications;

  // Load image
  this->logger.log("Loading image");
  cv::Mat image = cv::imread(imagePath);
  if (image.empty()) {
    this->logger.log("Error loading image" + imagePath.string());
    LOG(FATAL) << "Error: Could not load image.";
    return classifications;
  }
  this->logger.log("Image loaded");
  // Encode image as JPG
  this->logger.log("Compressing image");
  std::vector<uchar> encoded_image;
  if (!cv::imencode(".jpg", image, encoded_image)) {
    LOG(FATAL) << "Error: Image encoding failed.";
    return classifications;
  }
  this->logger.log("Image Compressed");

  // Get image size
  uint64_t img_size = encoded_image.size();

  // Create ZeroMQ message
  zmqpp::message message;
  message << img_size;                             // First frame: image size
  message.add_raw(encoded_image.data(), img_size); // Second frame: image bytes

  try {
    this->logger.log("Sending image to model");
    this->requestSocket.send(message);
    this->logger.log("Image sent successfully. Bytes: " + std::to_string(img_size));

    // Receive OCR result
    zmqpp::message response;
    this->requestSocket.receive(response);

    if (response.parts() == 0) {
      this->logger.log("No response received from OCR server");
      LOG(FATAL) << "No response received from OCR server";
      return classifications;
    }

    std::string extractedText;
    response.get(extractedText, 0);
    this->logger.log("Received raw OCR result:" + extractedText);
    std::string foodClassification = "";
    std::string expirationDate     = "";
    try {
      if (extractedText.empty() || extractedText[0] != '{') {
        this->logger.log("Invalid JSON received: " + extractedText);
        LOG(FATAL) << "Invalid JSON received: " << extractedText;
        return classifications;
      }

      nlohmann::json formatText = nlohmann::json::parse(extractedText);
      if (formatText.contains("Food Labels") && formatText["Food Labels"].is_array()) {
        classifications.setFoodItems(
            formatText["Food Labels"].get<std::vector<std::string>>());
      }

      if (formatText.contains("Expiration Date") &&
          formatText["Expiration Date"].is_array()) {
        classifications.setExpirationDates(
            formatText["Expiration Date"].get<std::vector<std::string>>());
      }

    } catch (const nlohmann::json::parse_error& e) {
      LOG(FATAL) << "JSON Parse Error: " << e.what();
      return classifications;
    } catch (const std::exception& e) {
      LOG(FATAL) << "General Error: " << e.what();
      return classifications;
    }

    this->logger.log("Received classification result: " +
                     (classifications.getFoodItems().empty()
                          ? "None"
                          : joinVector(classifications.getFoodItems(), ", ")));
    this->logger.log("Received expiration date result: " +
                     (classifications.getExpirationDates().empty()
                          ? "None"
                          : joinVector(classifications.getExpirationDates(), ", ")));

    return classifications;
  } catch (const zmqpp::exception& e) {
    LOG(FATAL) << "ZeroMQ error: " << e.what();
    return classifications;
  }
}
