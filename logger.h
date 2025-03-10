#ifndef LOGGER_H
#define LOGGER_H
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <string>

class Logger {
private:
  static std::mutex globalMutex;
  static std::map<std::string, std::ofstream*> sharedFiles;
  static std::map<std::string, int> refCounts;

  std::string filename;
  std::ofstream* logFile;
  mutable std::mutex logMutex; // Make the mutex mutable
  bool isShared;

public:
  Logger(const std::string& logFilename, bool shared = true)
      : filename(logFilename), isShared(shared) {
    if (shared) {
      std::lock_guard<std::mutex> lock(globalMutex);

      // Check if we already have a file stream for this filename
      auto it = sharedFiles.find(filename);
      if (it != sharedFiles.end()) {
        // Use existing file stream
        logFile = it->second;
        refCounts[filename]++;
      }
      else {
        // Create new file stream
        logFile               = new std::ofstream(filename, std::ios::trunc);
        sharedFiles[filename] = logFile;
        refCounts[filename]   = 1;
      }
    }
    else {
      // Create a private file stream
      logFile = new std::ofstream(filename, std::ios::trunc);
    }
  }

  ~Logger() {
    if (isShared) {
      std::lock_guard<std::mutex> lock(globalMutex);

      refCounts[filename]--;
      if (refCounts[filename] == 0) {
        // Last reference, clean up
        if (logFile->is_open()) {
          logFile->close();
        }
        delete logFile;
        sharedFiles.erase(filename);
        refCounts.erase(filename);
      }
    }
    else {
      // Clean up private file
      if (logFile->is_open()) {
        logFile->close();
      }
      delete logFile;
    }
  }

  // Now this can be a const method
  void log(const std::string& message) const {
    // We can now lock the mutex in a const method because it's mutable
    std::lock_guard<std::mutex> lock(isShared ? globalMutex : logMutex);

    if (logFile && logFile->is_open()) {
      // Get current time
      time_t now = time(nullptr);
      char timeStr[26];
      ctime_r(&now, timeStr);
      timeStr[24] = '\0'; // Remove newline

      // Write the log entry
      *logFile << "[" << timeStr << "] " << message << std::endl;
      logFile->flush();
    }
  }

  // Other methods can also be made const if appropriate
  bool isOpen() const {
    std::lock_guard<std::mutex> lock(isShared ? globalMutex : logMutex);
    return logFile && logFile->is_open();
  }
};

#endif
