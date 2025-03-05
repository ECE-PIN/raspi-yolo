#include "../include/visionMain.h"

/**
 * Entry into the vision code. Only called from main after vision child process is
 * forked.
 *
 * Input:
 * @param pipes Pipes for vision to communicate with the other processes
 * Output: None
 */
void visionEntry(struct Pipes pipes) {
  LOG(INFO) << "Within vision process";
  closeUnusedPipes(pipes);

  int maxRetries = 5;
  int retryCount = 0;

  while (!startPythonServer()) {
    retryCount++;

    if (retryCount >= maxRetries) {
      LOG(FATAL) << "ERROR: Failed to start Python server after " << retryCount
                 << " attempts.";
      exit(1); // Stop the program if the server cannot start
    }

    LOG(WARNING) << "Python server failed to start. Retrying in 2 seconds... ("
                 << retryCount << "/" << maxRetries << ")";
    std::this_thread::sleep_for(
        std::chrono::seconds(2)); // Wait 2 seconds before retrying
  }

  LOG(INFO) << "Waiting for Python server to be ready...";
  if (!isPythonServerReady()) {
    LOG(FATAL) << "Python server did not become ready in time.";
    exit(1);
  }

  struct FoodItem foodItem;
  ImageProcessor processor = ImageProcessor(pipes, foodItem);
  // Wait for start signal from Display with 0.5sec sleep
  while (1) {
    LOG(INFO) << "Waiting for start signal from Hardware";
    if (receiveFoodItem(foodItem, pipes.hardwareToVision[READ], (struct timeval){1, 0})) {
      LOG(INFO) << "Vision Received all images from hardware";
      processor.setFoodItem(foodItem);
      processor.process();
    }
    else {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  }
}

/**
 * Start the python server for hosting models
 *
 * @return success for fail
 */
bool startPythonServer() {
  pid_t pid = fork();

  if (pid == -1) {
    LOG(FATAL) << "ERROR: Failed to fork process: " << strerror(errno);
    return false;
  }
  if (pid == 0) { // Child process
    LOG(INFO) << "Starting Python server...";
    char* args[] = {(char*)"./models-venv/bin/python3",
                    (char*)"../vision/Models/server.py", nullptr};
    execvp(args[0], args);
    LOG(INFO) << "ERROR: execvp() failed to start Python server.";
    exit(1); // Exit child process if execvp fails
  }
  else { // Parent process
    LOG(INFO) << "Python server started with PID: " << pid;
    return true;
  }
}

/**
 * Start the python server for hosting models
 *
 * @return success for fail
 */
bool isPythonServerReady() {
  constexpr int MAX_RETRIES    = 50;
  constexpr int RETRY_DELAY_MS = 500;

  namespace fs = std::filesystem;

  for (int i = 0; i < MAX_RETRIES; ++i) {
    if (fs::exists(PIPE_OUT)) {
      return true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_DELAY_MS));
  }
  return false;
}
