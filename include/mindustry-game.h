#pragma once

#include "time-defaults.h"
#include <string>


class mindustry_game {
public:
    enum its_status { RUNNING, PAUSED, STOPPED, ENDED };

    mindustry_game(std::string game_name, its_status status = RUNNING,
	 time_point launched = std::chrono::system_clock::now()):
	status_(status), launched_(launched),
        hosted_game_(std::move(game_name)) {}

    std::string describe_game(bool stop = false);

private:
    its_status status_ = RUNNING;

    time_point launched_;
    std::string hosted_game_;

    std::string describe_status();
};

