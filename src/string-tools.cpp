#include "string-tools.h"

std::vector<std::string> split(const std::string &text, char sep) {
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

std::string join(const std::vector<std::string> &lines, char sep) {
    std::string joined_text;

    int index = 0;
    for (const auto &line: lines) {
	if (index > 0)
	    joined_text += sep;

	joined_text += line;
	++ index;
    }

    return joined_text;
}


// TODO: maybe make a generalized string replace? Or, at least, symbol to
//       string replace?
std::string translate_seps(const std::string &text, char from, char to) {
    // TODO: optimize this
    return join(split(text, from), to);
}
