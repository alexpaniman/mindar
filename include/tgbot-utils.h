#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>


using chat_id = int64_t;

struct message_id {
    chat_id from;
    int32_t id;
};

namespace tg {

    // Opaque class representing everything we need to know to interact with bot
    class bot;

    struct button { std::string name, callback; };

    struct message {
	std::string text;
	std::vector<std::vector<button>> inline_markup;

	message_id send_to(bot &api, int64_t chat_id) const;
	void edit_into(bot &api, message_id message) const;
    };

    struct sent_message {
	int64_t chat_id;
	int32_t message_id;

        std::string text;
    };

    class bot {
    public:
	bot(std::string token);

	virtual void on_any_message(sent_message msg) = 0;
	virtual void on_callback(sent_message msg, std::string callback) = 0;
	virtual void on_async_timer() = 0;

        [[noreturn]] void run();

        virtual ~bot() = default;

	// forbid bot copying:
	bot(const bot &other) = delete;
	bot& operator=(const bot &other) = delete;

    private:
        struct impl; // forward declare tgbot implementation details
        struct impl_deleter { void operator()(impl*) const; };

        std::unique_ptr<impl, impl_deleter> impl_;

        friend struct message; 
    };

}

