#include "mindustry-process.h"
#include "process-handling.h"
#include "time-defaults.h"

#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>


mindustry_server mindustry_server::launch() {
    // TODO: separate in global configuration:
    const auto TIMEOUT = 1s;

    // TODO: generalize JDK and server location!
    process server_process = create_process("java", { "-jar", "server.jar" });

    // Skip all initial boilerplate text Anuken
    // decided to add after server's launch:
    communicate_command(server_process, "", TIMEOUT);
    return server_process;
}


static std::vector<std::string> split(const std::string &text, char sep = ' ') {
    std::vector<std::string> strs;

    size_t pos = text.find(sep), init_pos = 0;
    while (pos != std::string::npos) {
        strs.push_back(text.substr(init_pos, pos - init_pos));
        init_pos = pos + 1;

        pos = text.find(sep, init_pos);
    }

    strs.push_back(text.substr(init_pos, std::min(pos, text.size()) - init_pos + 1));
    return strs;
}


static void ensure(std::stringstream &ss, const char *msg) {
    while (*msg != '\0') {
	char symbol;
	ss.read(&symbol, 1);

	if (symbol != *msg) {
	    // TODO: think where better to write logs!
	    std::cerr << "Parse error, expected: '" << *msg << "', got '" << symbol << "'\n";
	    _exit(-1); // TODO: better way than to exit?
	}

        ++ msg;
    }
}

mindustry_server::log_message mindustry_server::log_message::parse(std::stringstream &ss) {
    auto time = parse_time_stamp(ss, "[%m-%d-%Y %H:%M:%S]");
    ensure(ss, " [");

    log_message::its_kind kind;

    char symbol; ss >> symbol;
    switch (symbol) {
    case 'I':
	kind = log_message::INFO;
	break;

    case 'E':
	kind = log_message::ERROR;
	break;

    default:
	std::cerr << "Unexpected kind: " << symbol << "\n";
	_exit(-1);
    }

    ensure(ss, "] ");

    std::string line;
    std::getline(ss, line);

    return { kind, time, line };
}

std::vector<mindustry_server::log_message> mindustry_server::command(std::string cmd) {
    std::string output = communicate_command(server_process_, std::move(cmd) + "\n");

    // Remove all colors from mindustry output:
    output = filter_out_ansi_colors(output);

    if (output.back() == '\n')
	output.pop_back();

    std::vector<std::string> logs = split(output, '\n');

    std::vector<log_message> parsed_log;
    for (const auto &log: logs) {
	std::stringstream ss(log);
	parsed_log.emplace_back(mindustry_server::log_message::parse(ss));
    }

    return parsed_log;
}

std::vector<std::string> saves() { // NOTE: currently doesn't use any information from this class!
    std::vector<std::string> listed_saves;

    for (const auto &entry : std::filesystem::directory_iterator("./config/saves")) {
	if (entry.is_directory())
	    continue;

	std::string name = entry.path().filename();
	const char *extension = ".msav";

	if (name.ends_with(extension)) {
	    int len = name.size() - std::strlen(extension);
	    listed_saves.push_back(name.substr(0, len));
	}
    }

    return listed_saves;
}

