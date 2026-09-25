#include "Globals.h"
#include "core/RenderWindow.h"
#include "battle/Battle.h"
#include "core/Rng.h"

bool useMove(int input, Pokemon &my, Pokemon &op, bool isOpponent) {
	std::string newBattleSentence;
	if (isOpponent == true) newBattleSentence += "Opposing ";
	newBattleSentence += my.data->name + " used " + (my.data->move[input])->name + "!";
	mainBattle.battleDialogues.push_back(newBattleSentence);

	if (isOpponent) mainBattle.turnActionQueue.push_back("OPPONENT_USE_MOVE");
	else mainBattle.turnActionQueue.push_back("PLAYER_USE_MOVE");

	float STAB=1;
	if (my.data->move[input]->type==my.data->type||my.data->move[input]->type==my.data->stype) STAB=1.5;
	float TE=typeEffectiveness[my.data->move[input]->type][op.data->type]*typeEffectiveness[my.data->move[input]->type][op.data->stype];

	op.c_hp-=(my.data->move[input]->power*my.data->atk/op.data->def/2*STAB*TE)*randomInt(86, 100)/100;
	my.c_pp[input]--;
	if (op.c_hp<0) op.c_hp=0;

	if (TE==0) {
		if (!isOpponent) {
			mainBattle.battleDialogues.push_back("It didn't affect the opposing " + op.data->name + "...");
			mainBattle.turnActionQueue.push_back("MOVE_NOEFFECT");
		} else {
			mainBattle.battleDialogues.push_back("It didn't affect " + op.data->name + "...");
			mainBattle.turnActionQueue.push_back("MOVE_NOEFFECT");
		}
	}

	else if (TE<1) {
		mainBattle.battleDialogues.push_back("It's not very effective...");
		mainBattle.turnActionQueue.push_back("MOVE_NOT_EFFECTIVE");
	}

	else if (TE>1) {
		mainBattle.battleDialogues.push_back("It's super effective!");
		mainBattle.turnActionQueue.push_back("MOVE_SUPER_EFFECTIVE");
	}

	if (!op.c_hp) return true;
	return false;
}

float moveEvaluate(Move* move,Pokemon &my,Pokemon &op) {
	float STAB=1,TE;
	if (move->type==my.data->type||move->type==my.data->stype) STAB=1.5;
	TE=typeEffectiveness[move->type][op.data->type]*typeEffectiveness[move->type][op.data->stype];
	return move->power*TE*STAB;
}

int computerChooseMove(Pokemon &my,Pokemon &op) {
	if (randomInt(0, 1) == 0) {
		return randomInt(0, 3);
	} else {
		int r=0;
		for (int i=1;i<4;i++) {
			if (moveEvaluate(my.data->move[i],my,op)>moveEvaluate(my.data->move[r],my,op))
			r=i;
		}
		return r;
	}
}
