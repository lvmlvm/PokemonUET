#pragma once

#include "BattleScreen.h"
#include "mPlayer.h"
#include "Pokemon.h"

bool useMove(int input, Pokemon &my, Pokemon &op, bool isOpponent);

int computerChooseMove(Pokemon &my,Pokemon &op);

extern std::string Type[];
extern int psize;
extern BattleScreen mainBattle;
