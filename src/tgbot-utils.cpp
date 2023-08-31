#include "tgbot-utils.h"
#include "time-defaults.h"

#include <sys/types.h>

#include <tgbot/TgException.h>
#include <tgbot/tgbot.h>
#include <tgbot/types/CallbackQuery.h>
#include <tgbot/types/Message.h>


struct tg::bot::impl { TgBot::Bot bot; };

void tg::bot::impl_deleter::operator()(impl* impl) const {
    // just delete pointer, needed because std::unique_pointer<>
    // in some implementations statically asserts sizeof of type
    // is bigger than 0, and that doesn't work on incomplete ts

    delete impl;
}

tg::bot::bot(std::string token):
    impl_(new tg::bot::impl { TgBot::Bot(std::move(token)) }) {

    std::chrono::time_point start = std::chrono::system_clock::now();


    impl_->bot.getEvents().onAnyMessage([&](TgBot::Message::Ptr message) {
	sent_message sent {
            message->chat->id,
            message->messageId,
            message->text
	};

        on_any_message(std::move(sent));
    });

    impl_->bot.getEvents().onCallbackQuery([&](TgBot::CallbackQuery::Ptr callback) {
	sent_message sent {
            callback->message->chat->id,
            callback->message->messageId,
            callback->message->text
	};

        on_callback(std::move(sent), callback->data);
    });
}


void tg::bot::run() {
    const auto RESTART_DELAY = 1s;

    while (true) {
	std::chrono::time_point start = std::chrono::system_clock::now();

	auto &api = impl_->bot.getApi();
	try {
	    std::cout << "==> bot username: " << api.getMe()->username << "\n\n";

	    TgBot::TgLongPoll long_poll(impl_->bot);
	    while (true) {
		std::cout << "long poll initiated " << describe_passed_time_since(start) << "\n";
		long_poll.start();
	    }
	} catch (TgBot::TgException& e) {
	    std::cout << "error: " << e.what() << "\n\n";
	    std::cout << "==> trying to restart bot...\n";;

	    std::this_thread::sleep_for(RESTART_DELAY);
	}
    }
}


static TgBot::InlineKeyboardMarkup::Ptr keyboard(std::vector<std::vector<tg::button>> &keyboard) {
    auto inline_keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();

    auto &rows = inline_keyboard->inlineKeyboard;
    for (auto &row : keyboard) {
	rows.emplace_back();

	for (auto &[name, callback]: row) {
	    auto button = std::make_shared<TgBot::InlineKeyboardButton>();

            button->text = name;
            button->callbackData = callback;

	    rows.back().emplace_back(std::move(button));
	}
    }

    return inline_keyboard;
}


static void try_ignoring_errors(auto&& lambda) {
    try {
	std::forward<decltype(lambda)>(lambda)();
    } catch (TgBot::TgException &exc) {
	std::cout << "error: " << exc.what() << "\n";
    }
}

void tg::message::edit_into(bot &api, int64_t chat_id, int32_t message_id) {
    auto &internalApi = api.impl_->bot.getApi();
    auto kbd = keyboard(inline_markup);

    try_ignoring_errors([&]() {
	internalApi.editMessageText(
            message, chat_id, message_id, "",
	    "markdown", false, kbd
	);
    });
}

int32_t tg::message::send_to(bot &api, int64_t chat_id) {
    auto &internalApi = api.impl_->bot.getApi();

    auto kbd = keyboard(inline_markup);

    int32_t message_id = 0;
    try_ignoring_errors([&]() {
	message_id = internalApi.sendMessage(
	    chat_id, message, false,
	    0, kbd, "markdown"
	)->messageId;
    });

    return message_id;
}
