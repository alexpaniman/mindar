#pragma once

#include "process-handling.h"
#include "time-defaults.h"

#include <string>
#include <vector>


class mindustry_server {
public:
    static mindustry_server launch();

    struct log_message {
	enum its_kind { ERROR, INFO };

	its_kind kind;

	time_point date;
	std::string msg;

	static log_message parse(std::stringstream &ss);
    };

    std::vector<log_message> command(std::string cmd);

    std::vector<std::string> saves(); 

private:
    // TODO: generalize server location
    mindustry_server(process server_process):
	server_process_(server_process) {}

    process server_process_;
};    

