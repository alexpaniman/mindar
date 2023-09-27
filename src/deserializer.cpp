#include "deserializer.h"

#include <string.h>
#include <assert.h>

const inline char sep = '.';


std::string serialize_string(const std::string &string) {
    return std::to_string(string.size()) + sep + string;
}


static int parse_int(const char* &number_start, char terminator = ' ') {
    long current_number = 0;


    while (true) {
        char current_symbol = *number_start++;

        bool is_digit = '0' <= current_symbol && current_symbol <= '9';
	if (!is_digit)
	    break;

        int digit = current_symbol - '0';

        current_number *= 10;
        current_number += digit;
    }

    return current_number;
}  


std::string deserialize_string(const char* &serialized_data) {
    int length = parse_int(serialized_data);
    std::string parsed_string(serialized_data, serialized_data + length);

    serialized_data += length;
    return parsed_string;
}


std::vector<std::string> deserialize_vector(const char* &serialized_data) {
    int number_of_elements = parse_int(serialized_data);

    std::vector<std::string> parsed_values;
    for (int i = 0; i < number_of_elements; ++ i) {
        std::string parsed_string =
            deserialize_string(serialized_data);

        parsed_values.push_back(std::move(parsed_string));
    }

    return parsed_values;
}  

std::map<std::string, std::vector<std::string>> deserialize_map(const char* &serialized_data) {
    int number_of_entries = parse_int(serialized_data);

    std::map<std::string, std::vector<std::string>> parsed_entries;
    for (int i = 0; i < number_of_entries; ++ i) {
        std::string parsed_key                = deserialize_string(serialized_data);
        std::vector<std::string> parsed_value = deserialize_vector(serialized_data);

        parsed_entries.emplace(std::move(parsed_key), std::move(parsed_value));
    }


    return parsed_entries;
}


// TODO: make this whole thing safer!
