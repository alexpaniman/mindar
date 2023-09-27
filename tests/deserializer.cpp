#include "deserializer.h"

#include <iostream>


int main() {

    const char* string = "2.4.crux1.4., Hi7.sharded1.3.Hor";
    auto map = deserialize_map(string);

    for (auto &[key, values]: map) {
	std::cout << key << ": [ ";

        for (auto &value: values) {
	    std::cout << "\"" << value << "\"" << " ";
        }

        std::cout << "]\n";
    }
}
