#pragma once

#include "time-defaults.h"

#include <string>
#include <map>
#include <sstream>


struct team {
    // The name player selects when joining game. Beware, player can switch his name!
    // TODO: Consider using IP or other means of player identifications to make sure
    //       that game is going on between the same people.

    // TODO: first field is the name! 

    // This map maps player names to number of polling ticks at which this player was found  
    // playing for either of the sides.

    // The reason for such arrangment comes from the way in which we get information about 
    // teams and their players: through polling server every 10s or so and checking current
    // set of teams and players.

    // Tick can then be used to roughly estimate how much time player spent fighting for
    // either side and decide between whom this game was going on in general.

    // TODO: most of this should be abstracted away at some point, I don't like it now!
    std::map<std::string, size_t> players;
};


struct game_load {
    time_point load_time;
    std::optional<time_point> unload_time;

    duration duration() const {
	// Calculate total load duration. If unload_time is set to none, than current load
	// is still going and it's duration is calculated to current time. 
	return load_time - (unload_time ? *unload_time : std::chrono::system_clock::now());
    }      
};


enum class game_mode { SURVIVAL, SANDBOX, ATTACK, PVP };

// TODO: make enums with automatic name for god's sake... 
std::string get_game_mode_name(game_mode mode) {
    switch (mode) {
    case game_mode::SURVIVAL: return "survival";
    case game_mode::ATTACK:   return   "attack";
    case game_mode::SANDBOX:  return  "sandbox";
    case game_mode::PVP:      return      "pvp";
    }
}


// TODO: I don't like how this looks either
class mindustry_game {
public:
    // I want all "active" states to compare less to all "passive" states
    enum its_status { RUNNING = 0, PAUSED = 1, STOPPED = 2, ENDED = 3 };

    using id = size_t; // TODO: move to a more appropriate place

    mindustry_game(std::string game_name, its_status status = RUNNING,
                   time_point launched = std::chrono::system_clock::now()):
    status_(status), map_name_(std::move(game_name)) {

	game_load initial_load { launched, {} };
	loads_.emplace_back(std::move(initial_load));
    }

    void update_on_tick(const std::map<std::string, std::vector<std::string>> &current_team_set) {
	// Note every player we've seen, incrementing it's tick time:
	for (auto &[name, players]: current_team_set) {
	    for (auto &player: players)
		++ teams_[name].players[player];
	}
    }


    its_status        status() const { return   status_; }
    std::string     map_name() const { return map_name_; }

    time_point launched_time() const {
	// Get time of the first load --- it's the birth time of a game that is used for, among
	// other things, to identify it and describe different game loads.
        return loads_[0].load_time;
    }

    std::string describe_game() const {
	size_t estimated_game_duration_ticks = estimate_game_duration_ticks();

	std::stringstream description;

	description << "```";

        description << "======= general game info =======";
	description << "      map: " << map_name() << "(" << describe_status() << ")\n";
	description << "  created: " << describe_time_point(launched_time()) << "\n\n";

	description << "last load: " << describe_time_point(loads_.back().load_time) << "\n";
	description << " duration: " << describe_duration(total_game_duration(), true) << "\n\n";

        description << "============= teams =============";
        for (const auto &[name, team]: teams_) {
	    description << "(" << name << ")\n";

	    // TODO: sort players properly
            for (const auto &[name, time_spent_ticks]: team.players) {
		description << "    " << name << " ";

                if (time_spent_ticks >= estimated_game_duration_ticks * PLAYER_PERCENT)
		    description << "(guest)";

		description << "\n";
	    }

	    description << "\n";
	}

	description << "```";

        return description.str();
    }

    // TODO: Is this even a good idea to create artificial ordering to solve a particular problem.
    auto operator<=>(const mindustry_game &other) {
	// TODO: better let's sort by start time?
	return status() < other.status();
    }

    size_t calculate_id() const {
	// I'm assuming games are not created more frequently than 1 per second,
	// which would mean that time point can be used like a unique game id:

	const auto time_since_epoch = launched_time().time_since_epoch();
	const auto seconds = duration_cast<std::chrono::seconds>(time_since_epoch).count();

	return std::hash<int>{}(seconds);
    }

    std::string describe_game_brief() { // TODO: rename
        std::string message;

        message += describe_time_point(launched_time(), "%d/%m/%Y %H:%M");
	message += " ";

	message += describe_duration(total_game_duration());
	message += " ";

	message += describe_teams_brief();
    }

private:
    its_status status_ = RUNNING;
    std::vector<game_load> loads_;

    std::string map_name_;
    game_mode mode_;

    std::map<std::string, team> teams_;



    // If player was in game less than selected percent of game's duration,
    // than they are considered to be a guest player but not a main one:
    static inline const double PLAYER_PERCENT = 0.5;


    std::string describe_status() const;

    duration total_game_duration() const {
	duration total_duration {};

        for (auto &load: loads_)
	    total_duration += load.duration();

        return total_duration;
    }

    // Create short team description, for example:
    // ==> Prof & Garison vs Hor vs Etoy & Gari
    std::string describe_teams_brief() const {
	size_t estimated_game_duarion_ticks = estimate_game_duration_ticks();

	std::string description;

        int team_index = 0;
        for (const auto &[name, team]: teams_) {
	    if (team_index > 0)
		description += "  vs  ";

	    std::vector<std::string> team_players;
            for (const auto &[name, time_ticks] : team.players)
		if (time_ticks >= estimated_game_duarion_ticks * PLAYER_PERCENT)
		    team_players.push_back(name);

	    // Make a set of names playing in this team separated by '&'
            for (size_t i = 0; i < team_players.size(); ++ i) {
		if (i > 0)
		    description += " & ";

		description += team_players[i];
	    }

            ++ team_index;
        }

        return description;
    }

    size_t estimate_game_duration_ticks() const {
	// Game duration is estimated to be the same as longest playing player runtime:
	size_t max_time = 0;
	for (const auto &[name, team]: teams_) 
	    for (const auto &[name, time_ticks]: team.players)
		max_time = std::max(max_time, time_ticks);

        return max_time;
    }        
};
