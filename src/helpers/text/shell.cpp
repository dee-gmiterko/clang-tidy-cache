#include "helpers/text/shell.h"

#include <cctype>

std::vector<std::string> splitShellCommand(const std::string& command) {
  std::vector<std::string> words;
  std::string current;
  bool inWord = false;
  char quote = '\0';
  for (size_t index = 0; index < command.size(); ++index) {
    char character = command[index];
    if (quote == '\'') {
      if (character == '\'') {
        quote = '\0';
      } else {
        current += character;
      }
    } else if (character == '\\' && index + 1 < command.size() &&
               (quote == '\0' || command[index + 1] == '"' || command[index + 1] == '\\')) {
      current += command[++index];
      inWord = true;
    } else if (quote == '"') {
      if (character == '"') {
        quote = '\0';
      } else {
        current += character;
      }
    } else if (character == '\'' || character == '"') {
      quote = character;
      inWord = true;
    } else if (std::isspace(static_cast<unsigned char>(character)) != 0) {
      if (inWord) {
        words.push_back(current);
        current.clear();
        inWord = false;
      }
    } else {
      current += character;
      inWord = true;
    }
  }
  if (inWord) {
    words.push_back(current);
  }
  return words;
}
