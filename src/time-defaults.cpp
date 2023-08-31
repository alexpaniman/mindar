#include "time-defaults.h"


std::string describe_passed_time_since(time_point point) {
    auto now = std::chrono::system_clock::now();
    auto time_passed_seconds =
        std::chrono::duration_cast<std::chrono::seconds>(now - point).count();

    std::ostringstream oss;

    int   hours = time_passed_seconds / 3600;
    int minutes = time_passed_seconds % 3600 / 60;
    int seconds = time_passed_seconds % 60;

    oss << hours << " hrs " << minutes << " min " << seconds << " sec";
    return oss.str();
}

std::string describe_time_point(time_point point) {
    std::time_t time = std::chrono::system_clock::to_time_t(point);
    auto tm = std::localtime(&time);

    std::ostringstream oss;

    oss << std::put_time(tm, "%d-%m-%Y %H:%M:%S");
    return oss.str();
}

time_point parse_time_stamp(std::stringstream &ss, const char *format) {
    std::tm tm = {};
    ss >> std::get_time(&tm, format);

    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

