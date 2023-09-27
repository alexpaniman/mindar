#include "deserializer.h"
#include "mindustry-game.h"
#include "mindustry-process.h"
#include "tgbot-utils.h"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <linux/stat.h>
#include <map>
#include <sys/types.h>


struct mindustry_status_message {
    mindustry_game::id game_id;
    tg::message message;
};  


class mindar_bot: public tg::bot {
public:
    using tg::bot::bot;

    void on_async_timer() override {}
    void on_callback(tg::sent_message msg, std::string callback) override {
        const char *callback_it = callback.c_str();

        // TODO: make this deserialization more convenient!
        auto id = deserialize_string(callback_it);
        mindustry_game::id game_id = std::strtoull(id.c_str(), NULL, 10);


        // TODO: use game_id!

        auto command = deserialize_string(callback_it);

        if      (command == "pause")
            server_.pause_or_resume_current_game();
        else if (command ==  "stop")
            server_.stop_current_game();
    }      


    void on_any_message(tg::sent_message msg) override {
        if (msg.text != "/start")
            return;

    }

    // void send_status_message(const mindustry_game &game, chat_id chat) {
    //     const auto description_message = generate_status_message(game);

    //     if (status_messages_.contains(chat)) {
    //         auto suspended_message = description_message;

    //         // Add message a note to the message:
    //         suspended_message.text += "\n"
    //             "__This message's updates are suspended in favour of the later messages, "
    //             "this one will not be edited anymore.__";

    //         // Remove inline markup:
    //         suspended_message.inline_markup = {};

    //         suspended_message.edit_into(*this, status_messages_[chat].message);
    //     }

    //     status_messages_[chat] = {
    //       game.calculate_id(),
    //       description_message.send_to(*this, chat)
    //     };
    // }

    // void update_status_messages() {
    //     for (auto &[chat, stats_message] : status_messages_) {
    //         // TODO: maybe should somehow compare old contents to new contents and not
    //         //       edit if they match?
    //         const auto description = generate_status_message(stats_message.game_id);
    //         description.edit_into(*this, stats_message.message);
    //     }
    // }

private:
    mindustry_server server_;
    std::map<chat_id, mindustry_status_message> status_messages_;


    tg::message generate_status_message(const mindustry_game &game) {
        const size_t game_id = game.calculate_id();

        // TODO: adjust keyboard layout
        return tg::message {
            game.describe_game(), {
                {{  "save", serialize(game_id,  "save") }},
                {{ "pause", serialize(game_id, "pause") }},
                {{  "stop", serialize(game_id,  "stop") }},
            }
        };
    }


};  

int main() {
    std::cout << "MINDAR [Mindustry Interaction & Direct Authority Remote]\n";
    std::cout << "Copyright (c) Alex Paniman 2023\n\n";


    mindar_bot bot(getenv("BOT_TOKEN"));
    bot.run();
}
