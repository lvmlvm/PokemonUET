#pragma once

#include "scenes/BattleScreen.h"
#include "world/Player.h"
#include "battle/Pokemon.h"

bool useMove(int input, Pokemon &my, Pokemon &op, bool isOpponent);

int computerChooseMove(Pokemon &my,Pokemon &op);

extern std::string Type[];
extern int psize;
extern BattleScreen mainBattle;
