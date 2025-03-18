#include "../include/helperFunctions.h"

/**
 * Determine if a path is valid directory
 *
 * Input:
 * @param directory path to directory
 * Output: bool on whether is a valid directory
 */
bool isValidDirectory(const std::filesystem::path& directory) {
  if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
    return false;
  }
  return true;
}

/**
 * Determine if a given directory has files
 *
 * Input:
 * @param dirPath path to directory to examine
 * Output: bool on whether or not directory has files
 */
bool hasFiles(const std::filesystem::path& dirPath) {
  return std::filesystem::exists(dirPath) && std::filesystem::is_directory(dirPath) &&
         !std::filesystem::is_empty(dirPath);
}

/**
 * convert string to all lowercase
 *
 * Input:
 * @param str string to convert
 * @return lowercase string
 */
std::string toLowerCase(std::string str) {
  std::transform(str.begin(), str.end(), str.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return str;
}

/**
 * split long strings into individual words
 *
 * Input:
 * @param text text to split apart
 * Output:
 * @return vector of strings
 */
std::vector<std::string> splitWords(const std::string& text) {
  std::vector<std::string> words;
  std::istringstream iss(text);
  std::string word;
  while (iss >> word) {
    words.push_back(word);
  }
  return words;
}

/**
 * Remove special characters
 *
 * Input:
 * @param text text clean
 * Output:
 * @return cleaned text
 */
std::string cleanText(const std::string& text) {
  std::string result;
  for (char c : text) {
    if (std::isalnum(c) || std::isspace(c)) {
      result += c;
    }
  }
  return result;
}

/**
 * Remove provided prefix if found
 *
 * Input:
 * @param string original string
 * @param prefix prefix to remove
 * Output:
 * @return updated string with prefix removed
 */
std::string removePrefix(const std::string& string, const std::string& prefix) {
  if (string.rfind(prefix, 0) == 0) {
    return string.substr(prefix.length());
  }
  return string;
}

/**
 * Joins the past vector to a single string separated by given delimiter. Default delim:
 * ", "
 *
 * Input:
 * @param vec original vector to join
 * @param delimiter delimiter to separate strings
 * Output:
 * @return updated string with prefix removed
 */
std::string joinVector(const std::vector<std::string>& vec,
                       const std::string& delimiter) {
  if (vec.empty())
    return "";

  std::ostringstream oss;
  auto it = vec.begin();
  oss << *it; // Add first element

  for (++it; it != vec.end(); ++it) {
    oss << delimiter << *it;
  }
  return oss.str();
}
