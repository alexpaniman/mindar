#include "mindustry-game.h"

#include <sstream>

std::string mindustry_game::describe_status() const {
    std::ostringstream message;

    switch (status_) {
    case RUNNING:
	message << "running";
	break;

    case PAUSED:
	message << "paused";
	break;

    case STOPPED:
	message << "stopped";
	break;

    case ENDED:
	message << "ended";
	break;
    }

    return message.str();
}


std::string mindustry_game::describe_game() const {
    std::ostringstream message;
    message << "*======== Mindustry Hosted ========*\n\n";

    message << "```";

    message << " ⏱️ time: "  << describe_time_point(       launched_time_) <<  "\n";
    message << "          (" << describe_passed_time_since(launched_time_) << ")\n";
    message << " 🌍  map: "  << map_name_                                <<  "\n";
    message << " 🗒️ stat: "  << describe_status();


    message << "```" << "\n";

    return message.str();
}
