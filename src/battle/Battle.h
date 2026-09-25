#pragma once

#include "battle/Pokemon.h"

#include <string>
#include <vector>

// What happened in a turn: one dialogue line per action, shown one at a time by the battle scene.
struct BattleLog {
	std::vector<std::string> dialogues;
	std::vector<std::string> actions;

	void add(const std::string& dialogue, const std::string& action) {
		dialogues.push_back(dialogue);
		actions.push_back(action);
	}
	void clear() {
		dialogues.clear();
		actions.clear();
	}
};

// Applies move `input` of `my` to `op` and logs it; returns true if `op` fainted.
bool useMove(BattleLog& log, int input, Pokemon &my, Pokemon &op, bool isOpponent);

int computerChooseMove(Pokemon &my,Pokemon &op);
