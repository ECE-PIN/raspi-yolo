#include <filesystem>
#include <fstream>
#include <glog/logging.h>
#include <iostream>
#include <string>
#include <sys/select.h>

#include "../../food_item.h"
#include "../../pipes.h"
#include "hardware_entry.h"

/**
 * Checks to see if there is any data fromDisplay[READ]
 * Used to initialize control system and begin scanning
 *
 * Input:
 * @param pipeToRead Pipe to read signal from Display button press
 *
 * Output:
 * @return Boolean; true or false
 */
bool receivedStartSignal(int pipeToRead) {
  fd_set readPipeSet;

  FD_ZERO(&readPipeSet);
  FD_SET(pipeToRead, &readPipeSet);

  struct timeval timeout;
  timeout.tv_sec  = 1;
  timeout.tv_usec = 0;

  // Check pipe for data
  int startSignal = select(pipeToRead + 1, &readPipeSet, NULL, NULL, &timeout);

  if (startSignal == -1) {
    LOG(FATAL) << "Select error when checking for start signal";
  }
  if (startSignal == 0) {
    LOG(INFO) << "No start signal received";
    return false;
  }
  else {
    LOG(INFO) << "Received start signal from display";
    std::string startSignal = readString(pipeToRead);
    return true;
  }
}

/**
 * Sends the photo directory and weight data to the AI Vision system.
 *
 * @param weight The weight of the object on the platform.
 * @return None
 */
void sendDataToVision(int pipeToWrite, float weight) {
  LOG(INFO) << "Sending Images from Hardware to Vision";
  const std::chrono::time_point<std::chrono::system_clock> now{
      std::chrono::system_clock::now()};

  struct FoodItem foodItem;
  foodItem.imageDirectory = std::filesystem::absolute("../images/temp");
  foodItem.scanDate       = std::chrono::floor<std::chrono::days>(now);
  foodItem.weight         = weight;
  sendFoodItem(foodItem, pipeToWrite);

  LOG(INFO) << "Done Sending Images from Hardware to Vision";
}

/**
 * Method to send images from a given directory using the pipe provided. It cycles through
 * all .jpg images within the directory and sends them across processes.
 *
 * Input:
 * @param pipeToWrite Pipe to write image information to
 * @param directory_path Directory path as a string to get images from
 * Output: None
 */
void sendImagesWithinDirectory(int pipeToWrite, const std::string& directory_path) {
  for (const auto& entry : std::filesystem::directory_iterator(directory_path)) {
    if (entry.is_regular_file()) {
      const std::string file_path = entry.path().string();

      // Check if the file has a .jpg extension
      if (entry.path().extension() != ".jpg") {
        continue; // Skip non-.jpg files
      }

      // Open the file
      std::ifstream file(file_path, std::ios::binary | std::ios::ate);
      if (!file.is_open()) {
        LOG(FATAL) << "Failed to open file: " << file_path;
        continue;
      }

      // Get the file size
      std::streamsize file_size = file.tellg();
      file.seekg(0, std::ios::beg);

      // Header: Send the file size (4 bytes)
      uint32_t header = static_cast<uint32_t>(file_size);
      write(pipeToWrite, &header, sizeof(header)); // Send file size

      // Send the file data in chunks
      const int CHUNK_SIZE = 4096;
      char buffer[CHUNK_SIZE];
      while (file_size > 0) {
        std::streamsize to_read =
            std::min(file_size, static_cast<std::streamsize>(CHUNK_SIZE));
        file.read(buffer, to_read);
        write(pipeToWrite, buffer, to_read);
        file_size -= to_read;
      }

      file.close();
    }
  }

  // Send a special header with size 0 to signal end of transmission
  uint32_t end_signal = 0;
  write(pipeToWrite, &end_signal, sizeof(end_signal));
}
