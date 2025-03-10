#include "logger.h"

std::mutex Logger::globalMutex;
std::map<std::string, std::ofstream*> Logger::sharedFiles;
std::map<std::string, int> Logger::refCounts;
