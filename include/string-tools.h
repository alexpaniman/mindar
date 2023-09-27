#pragma once

#include <vector>
#include <string>


std::vector<std::string> split(const std::string &text, char sep = ' ');
std::string join(const std::vector<std::string> &lines, char sep = ' ');

std::string translate_seps(const std::string &text, char from = '\n', char to = ' ');
