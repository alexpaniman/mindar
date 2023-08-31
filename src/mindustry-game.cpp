#include "mindustry-game.h"

#include <sstream>

std::string mindustry_game::describe_status() {
    std::ostringstream message;

    switch (status_) {
    case RUNNING:
	message << "▶️ run";
	break;

    case PAUSED:
	message << "⏸ paused";
	break;

    case STOPPED:
	message << "⏹️ stopped";
	break;

    case ENDED:
	message << "📙 ended";
	break;
    }

    return message.str();
}


std::string mindustry_game::describe_game(bool stop) {
    std::ostringstream message;
    message << "*======== Mindustry Hosted ========*\n\n";

    message << "```";

    message << " ⏱️ time: "  << describe_time_point(       launched_) <<  "\n";
    message << "          (" << describe_passed_time_since(launched_) << ")\n";
    message << " 🌍  map: "  << hosted_game_                                <<  "\n";
    message << " 🗒️ stat: "  << describe_status();


    message << "```" << "\n";

    return message.str();
}
