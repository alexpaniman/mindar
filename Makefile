mindar:
	clang++ --std=c++20 mindustry-bot.cpp -o mindar -lTgBot -lssl -lcrypto -O2

mindar.debug:
	clang++ --std=c++20 mindustry-bot.cpp -o mindar.debug -lTgBot -lssl -lcrypto -O0 -g3
