#pragma once


// TODO: rename, mindustry server!

#include "deserializer.h"
#include "mindustry-game.h"
#include "process-handling.h"
#include "string-tools.h"
#include "time-defaults.h"

#include <exception>
#include <string>
#include <vector>
#include <cassert>


class mindustry_server {
public:
    static mindustry_server launch();

    void update_game_teams() {
        std::string get_teams_js = R"(
	    function serialize_number(number) { return number.toString() + "."; }

	    function serialize_string(string) {
		return serialize_number(string.length) + string;
	    }

	    function serialize_stream(stream) {
		return serialize_number(stream.size) + stream.toString("");
	    }

	    function serialize_string_stream(stream) {
		return serialize_stream(stream.map(element => serialize_string(element)));
	    }

	    serialize_stream(
		Vars.state.teams.active.map(team =>
		    serialize_string(team.team.toString()) +
			serialize_string_stream(
			    team.players.map(player => player.name))));
	)";

        // Make it single line, mindustry server doesn't accept it any other way:
        get_teams_js = translate_seps(get_teams_js, '\n', ' ');
	auto result = command("js " +  get_teams_js);

	// TODO: make this from asserts to some kind of runtime error handling,
	//       notify bot owner (which should be known to bot):
	assert(result.size() == 1);
        assert(result.back().kind == log_message::INFO);

	std::string response = result.back().msg;
	const char* response_iterator = response.c_str();

        auto deserialized_map = deserialize_map(response_iterator);
	games_[current_game_id_].update_on_tick(deserialized_map);
    }

    // TODO: maybe pause and a separate resume is better?
    void pause_or_resume_current_game() {
	auto &current_game = games_[current_game_id_];

	assert(current_game.status() != mindustry_game::STOPPED);
        assert(current_game.status() != mindustry_game::ENDED);

	if (current_game.status() == mindustry_game::PAUSED)
	    command("pause on");
	else
	    command("pause off");
    }

    // TODO: any variable to check if hosting game now?
    void stop_current_game() { command("stop"); }

    // Beware, do not store reference to this game for long!
    const mindustry_game &game() { return games_[current_game_id_]; }

private:
    process server_process_;

    mindustry_game::id current_game_id_;
    std::map<mindustry_game::id, mindustry_game> games_;


    // TODO: generalize server location
    mindustry_server(process server_process):
	server_process_(server_process) {}


    struct log_message { // TODO: probably extract? I have a strong feeling that there can be two abstractions!
	enum its_kind { ERROR, INFO };

	its_kind kind;

	time_point date;
	std::string msg;

	static log_message parse(std::stringstream &ss);
    };


    // TODO: report in case any error messages appear!
    std::vector<log_message> command(std::string cmd);

    // TODO: is this needed? Since saves are now mostly stored in games_
    std::vector<std::string> saves(); 

};    

