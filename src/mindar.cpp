#include "tgbot-utils.h"
#include <iostream>


class mindar_bot: public tg::bot {
public:
    using tg::bot::bot;

    void on_async_timer() override {}
    void on_callback(tg::sent_message msg, std::string callback) override {}      


    void on_any_message(tg::sent_message msg) override {
	if (msg.text != "/start")
	    return;

	// TODO: in progress...
        tg::message {
            "Hi!", {
		{{ "click", "click" }},
		{{    "me",    "me" }},
            }
	}.send_to(*this, msg.chat_id);            
    }
};  

int main() {
    std::cout << "MINDAR [Mindustry Interaction & Direct Authority Remote]\n";
    std::cout << "Copyright (c) Alex Paniman 2023\n\n";


    mindar_bot bot(getenv("BOT_TOKEN"));
    bot.run();
}
