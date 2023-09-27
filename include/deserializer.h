#pragma once

#include <map>
#include <string> 
#include <vector>



std::string serialize_string(const std::string &string);

// Verson of serialize string that works for not just string, but also
// for everything that can be converted into one by std::to_string()
std::string serialize_string(auto &&string) {
    return serialize_string(std::to_string(std::forward<decltype(string)>(string)));
}


// Simple thing that can serialize multiples of objects one by one:
std::string serialize(auto &&...values) {
    return (serialize_string(std::forward<decltype(values)>(values)) + ...);
}



std::string deserialize_string(const char* &serialized_data);

std::vector<std::string> deserialize_vector(const char* &serialized_data);

std::map<std::string, std::vector<std::string>> deserialize_map(const char* &serialized_data);
