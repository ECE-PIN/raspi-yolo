#include "../include/IModel.h"
/**
 * Send request to models
 *
 * @param taskType Which model do you want to access
 * @param imagePath path to the image to look at
 * @return Response from the server
 */
std::string IModel::sendRequest(const TaskType& taskType,
                                const std::filesystem::path& imagePath) {
  this->logger.log("Enter sendRequest");

  std::string request = taskTypeToString(taskType) + " " + imagePath.string();

  this->logger.log("Sending request: " + request);
  std::string response;
  this->requestSocket.send(request);

  this->logger.log("Request sent, waiting for response");

  this->requestSocket.receive(response);
  this->logger.log("Got response from server: " + response);

  this->logger.log("Exiting sendRequest");
  return response;
}

std::string IModel::taskTypeToString(const TaskType& taskType) const {
  this->logger.log("Converting task type to string");
  switch (taskType) {
  case CLS:
    return "CLS";
  case OCR:
    return "OCR";
  case EXP:
    return "EXP";
  case unknown:
  default:
    return "unknown";
  }
}

void IModel::connectToServer(std::string& serverAddress) {
  this->requestSocket.connect(serverAddress);
}
