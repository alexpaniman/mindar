CXX=clang
CXXFLAGS=--std=c++20 -O2

LDFLAGS=-lTgBot -lssl -lcrypto
INCLUDE=-I include/

BUILD=build


$(BUILD)/mindar: $(BUILD)/mindustry-bot.o $(BUILD)/process-handling.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@


# ==> Stamps to track changes in object files:

$(BUILD)/process-handling.o: src/process-handling.cpp $(BUILD)/process-handling.h.stamp | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $< -c -o $@

$(BUILD)/mindustry-bot.o:    src/mindustry-bot.cpp    $(BUILD)/process-handling.h.stamp | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $< -c -o $@


# ==> Stamps to track changes in includes:

$(BUILD)/process-handling.h.stamp: include/process-handling.h $(BUILD)/time-defaults.h.stamp | $(BUILD)
	touch $@

$(BUILD)/mindustry-game.h.stamp:   include/mindustry-game.h   $(BUILD)/time-defaults.h.stamp | $(BUILD)
	touch $@

$(BUILD)/time-defaults.h.stamp:    include/time-defaults.h                                   | $(BUILD)
	touch $@



$(BUILD):
	mkdir -p $(BUILD)
