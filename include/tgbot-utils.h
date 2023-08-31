#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>


namespace tg {

    // Opaque class representing everything we need to know to interact with bot
    class bot;

    struct button { std::string name, callback; };

    using message_id = int;
    struct message {
	std::string message;
	std::vector<std::vector<button>> inline_markup;

	int32_t send_to(bot &api, int64_t chat_id);
	void edit_into(bot &api, int64_t chat_id, int32_t message_id);
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

        friend class message; 
    };

}

