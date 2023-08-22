#include <chrono>
#include <concepts>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdio.h>
#include <tgbot/types/GenericReply.h>
#include <tgbot/types/InlineKeyboardButton.h>
#include <tgbot/types/InlineKeyboardMarkup.h>
#include <tgbot/types/Message.h>
#include <tgbot/types/ReplyKeyboardMarkup.h>
#include <thread>
#include <unistd.h>
#include <utility>
#include <assert.h> 
#include <string>
#include <iostream>
#include <map> 
#include <fcntl.h>
#include <sys/wait.h>
#include <tgbot/tgbot.h>


using namespace std::chrono_literals;


struct process {
    pid_t pid;

    int  stdin_pipe_fileno;
    int stdout_pipe_fileno;
};

std::string filter_out_ansi_colors(std::string colored_text) {
    std::string filtered_text;

    for (int i = 0; i < colored_text.size(); ++ i) {
        if (colored_text[i] == '\033') {
	    while (colored_text[i] != 'm')
		++ i;

	    continue;
        }

	filtered_text += colored_text[i];
    }

    return filtered_text;
}

process create_process(const char *path, auto&&... args) {
    int in_fd[2], out_fd[2];

    pipe(in_fd);  // For child's stdin
    pipe(out_fd); // For child's stdout

    pid_t pid = fork();

    if (pid == 0) { // We're in the child process:
	std::cout << "Launching: " << path;
	((std::cout << " " << args), ...) << "\n";

	close(out_fd[0]);
	dup2(out_fd[1], STDOUT_FILENO);
	close(out_fd[1]);

	close(in_fd[1]);

	dup2(in_fd[0], STDIN_FILENO);
	close(in_fd[0]);

	execlp(path, path, args...);
	_exit(0);
    }

    else if (pid == -1) {
	perror("fork");
	_exit(-1);
    }
    else {
	// You're in the parent
	close(out_fd[1]);
	close(in_fd[0]);

	// Make interactions with pipe non-blocking. 
        int process_out = out_fd[0];

	int flags = fcntl(process_out, F_GETFL, 0);
	fcntl(process_out, F_SETFL, flags | O_NONBLOCK);

        return { pid, in_fd[1], process_out };
    }

    assert(false);
}


using duration   = std::chrono::duration<double>;
using time_point = std::chrono::time_point<std::chrono::system_clock>;

std::string communicate_command(process proc, std::string command = "", duration timeout = 0.15s) {
    if (write(proc.stdin_pipe_fileno, &*command.begin(), command.size()) < 0) {
	perror("write");
	_exit(-1);
    }

    const std::size_t READ_SIZE = 4096;
    char buffer[READ_SIZE + 1] = {};

    std::string response;
    time_point start_time = std::chrono::system_clock::now();

    const auto RESPONSE_WAIT = 0.001s;

    int last_read_bytes = 0;
    while (true) {
	time_point now = std::chrono::system_clock::now();
        if (now - start_time >= timeout)
	    break;

	int read_bytes = read(proc.stdout_pipe_fileno, buffer, READ_SIZE);
        if (read_bytes < 0) {
	    if (errno != EAGAIN) {
		perror("read");
		_exit(-1);
	    }

	    if (last_read_bytes > 0)
		memset(buffer, '\0', last_read_bytes);
        }

        last_read_bytes = read_bytes;
	buffer[read_bytes] = '\0';

	std::this_thread::sleep_for(RESPONSE_WAIT);
	response += buffer;
    }

    return response;
}


TgBot::InlineKeyboardMarkup::Ptr keyboard(std::initializer_list<std::initializer_list<std::string>> keyboard) {
    auto inline_keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();

    auto &rows = inline_keyboard->inlineKeyboard;
    for (auto &row : keyboard) {
	rows.emplace_back();

	for (auto &elements: row) {
	    auto button = std::make_shared<TgBot::InlineKeyboardButton>();

            button->text = elements;
            button->callbackData = elements;

	    rows.back().emplace_back(std::move(button));
	}
    }

    return inline_keyboard;
}

std::vector<std::string> saves() {
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


    // bot.getEvents().onAnyMessage([&](TgBot::Message::Ptr message) {
    //     printf("User wrote %s\n", message->text.c_str());

    // 	std::string response = communicate_command(proc, message->text + "\n");

    // 	std::ofstream of("log");
    // 	of << response;
    // 	of.flush();

    // 	char *args[] = { NULL };
    //     process creator = create_process("/home/alex/test/mindustry-bot-cpp/create-image.sh", args);

    //     int status;
    // 	waitpid(creator.pid, &status, 0);

    //     TgBot::InputFile::Ptr photo = TgBot::InputFile::fromFile("/home/alex/test/mindustry-bot-cpp/image.png", "image/png");
    //     bot.getApi().sendPhoto(message->chat->id, photo);
    // });








std::vector<std::string> split(const std::string &text, char sep = ' ') {
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



struct log_message {
    enum its_kind { ERROR, INFO };

    its_kind kind;

    time_point date;
    std::string msg;
};


time_point parse_time_stamp(std::stringstream &ss) {
    std::tm tm = {};
    ss >> std::get_time(&tm, "[%m-%d-%Y %H:%M:%S]");

    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

void ensure(std::stringstream &ss, const char *msg) {
    while (*msg != '\0') {
	char symbol;
	ss.read(&symbol, 1);

	if (symbol != *msg) {
	    std::cerr << "Parse error, expected: '" << *msg << "', got '" << symbol << "'\n";
	    _exit(-1);
	}

        ++ msg;
    }
}


log_message parse_log(std::stringstream &ss) {
    auto time = parse_time_stamp(ss);
    ensure(ss, " [");

    log_message::its_kind kind;

    char symbol; ss >> symbol;
    switch (symbol) {
    case 'I':
	kind = log_message::INFO;
	break;

    case 'E':
	kind = log_message::INFO;
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

std::vector<log_message> command_mindustry(process mindustry_process, std::string cmd) {
    std::string output = communicate_command(mindustry_process, std::move(cmd) + "\n");

    // Remove all colors from mindustry output:
    output = filter_out_ansi_colors(output);

    if (output.back() == '\n')
	output.pop_back();

    std::vector<std::string> logs = split(output, '\n');

    std::vector<log_message> parsed_log;
    for (const auto &log: logs) {
	std::stringstream ss(log);
	parsed_log.emplace_back(parse_log(ss));
    }

    return parsed_log;
}


struct game {
    time_point launched;

    std::string hosted_game;
    bool paused;

    std::map<int64_t, int32_t> messages;

    game(std::string game_name):
	launched(std::chrono::system_clock::now()),
	hosted_game(game_name),
	paused(false), messages() {}
};


std::string describe_game(const game &described_game, bool stop = false) {
    std::time_t time = std::chrono::system_clock::to_time_t(described_game.launched);
    auto tm = *std::localtime(&time);

    std::ostringstream message;
    message << "*======== Mindustry Hosted ========*\n\n" << "```";
    message << " ⏱️ time: " << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "\n";

    auto time_passed =
	std::chrono::duration_cast<std::chrono::seconds>(
	    std::chrono::system_clock::now() - described_game.launched).count();

    int hours = time_passed / 3600, minutes = time_passed % 3600 / 60, seconds = time_passed % 60;

    message << "          (" << hours << " hrs " << minutes << " min " << seconds << " sec)\n";
    message << " 🌍  map: " << described_game.hosted_game << "\n";
    message << " 🗒️ stat: "
	    << (stop? "⏹️ stopped" : !described_game.paused ? "▶️ run" : "⏸ paused") << "```\n";

    return message.str();
}

void send_stats_message(const TgBot::Api &api, int64_t chat_id, game &current_game) {
    auto keys = keyboard({
	{ "save"  },
	{ "pause" },
	{ "stop"  },
    });

    auto message = api.sendMessage(
        chat_id, describe_game(current_game), false,
	0, keys, "markdown");

    current_game.messages.insert({ chat_id, message->messageId });
}

void update_message(const TgBot::Api &api, const game &current_game, int64_t chat_id, int32_t message_id, bool stop = false) {
    auto keys = keyboard({
	{ "save"  },
	{ "pause" },
	{ "stop"  },
    });

    try {
        api.editMessageText(describe_game(current_game, stop), chat_id,
	    message_id, "", "markdown",
	    false, !stop? keys : keyboard({}));
    } catch (std::exception &ignore) { /* just don't edit */ }
}

void update_messages(const TgBot::Api &api, const game &current_game) {
    std::string description = describe_game(current_game);
    for (auto &[chat_id, message_id]: current_game.messages)
	update_message(api, current_game, chat_id, message_id);
}

std::string format_time(time_point moment, const char* format) {
    std::time_t now_time = std::chrono::system_clock::to_time_t(moment);
    std::tm *now_tm = std::localtime(&now_time);

    std::ostringstream oss;

    oss << std::put_time(now_tm, format);
    return oss.str();
}

int main() {
    std::cout << "MINDAR [Mindustry Interaction & Direct Authority Remote]\n";
    std::cout << "Copyright (c) Alex Paniman 2023\n\n";

    auto proc = create_process("java", "-jar", "server.jar");

    // Skip initialization text: 
    communicate_command(proc, "", 1s);


    TgBot::Bot bot(getenv("BOT_TOKEN"));

    auto &events = bot.getEvents();
    auto &api = bot.getApi();


    std::optional<game> current_game;

    events.onCommand("stats", [&](TgBot::Message::Ptr msg) {
        if (!current_game) {
	    api.sendMessage(msg->chat->id, "Not hosting, no stats to show!");
	    return;
        }

	send_stats_message(api, msg->chat->id, *current_game);
    });

    events.onCommand("host", [&](TgBot::Message::Ptr msg) {
	auto args = split(msg->text);
        if (args.size() != 2) {
	    api.sendMessage(msg->chat->id, "Not enought args, use /host MAP");
	    return;
        }

        if (current_game) {
	    api.sendMessage(msg->chat->id, "Already hosting! Stop game at first!");
	    return;
        }

	std::string map_name = args[1];
	std::cout << msg->text << "\n";

        command_mindustry(proc, std::string("host ") + map_name);

        current_game = game { map_name };
	send_stats_message(api, msg->chat->id, *current_game);
    });

    events.onCallbackQuery([&](TgBot::CallbackQuery::Ptr callback) {
	std::string data = callback->data;

        int64_t chat_id = callback->message->chat->id;

        if (data == "pause") {
            if (!current_game) {
		api.sendMessage(chat_id, "Not hosting a game right now!");
		return;
	    }

            if (current_game->paused)
		command_mindustry(proc, "pause off");
            else
		command_mindustry(proc, "pause on");

            current_game->paused = !current_game->paused;
        }

        else if (data == "stop") {
            if (!current_game) {
		api.sendMessage(chat_id, "Not hosting a game right now!");
		return;
            }

	    command_mindustry(proc, "stop");

	    update_message(api, *current_game, chat_id, callback->message->messageId, true);
	    current_game = {};
            return;
        }

        else if (data == "save") {
            if (!current_game) {
		api.sendMessage(chat_id, "Not hosting a game right now!");
		return;
            }

	    time_point now = std::chrono::system_clock::now();


	    // Format mindustry save record:
	    const char *time_format = "%d-%m-%Y+%H:%M:%S";

            std::ostringstream ss;

            ss << "save ";
            ss << format_time(current_game->launched, time_format) << "+";
            ss << current_game->hosted_game << "+";
	    ss << format_time(now, time_format);

            std::cout << ss.str() << "\n";

            command_mindustry(proc, ss.str());
        }

	update_message(api, *current_game, chat_id, callback->message->messageId);
    });


    std::chrono::time_point start = std::chrono::system_clock::now();

    try {
	std::cout << "Bot username: " << api.getMe()->username << "\n\n\n";

        TgBot::TgLongPoll long_poll(bot);
        while (true) {

	    std::chrono::time_point now = std::chrono::system_clock::now();
	    auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>(now - start).count() / 1e9;

            std::cout << "Long poll initiated: " << std::fixed << std::setw(9)
		      << std::setprecision(2) << delta << " seconds uptime\n";


            long_poll.start();

	    if (current_game) {
		update_messages(api, *current_game);
	    }
        }
    } catch (TgBot::TgException& e) {
	printf("error: %s\n", e.what());
    }
}
