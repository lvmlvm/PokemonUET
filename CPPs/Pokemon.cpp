#include "Pokemon.h"
#include <ctime>
#include <SDL.h>

Pokemon::Pokemon(int i) {
	data=pokemonData+i;
	c_hp=data->hp;
	for (int i=0;i<4;i++) {
		c_pp[i]=data->move[i]->pp;
	}
}

Pokemon::Pokemon() {
	data=pokemonData;
	c_hp=data->hp;
	for (int i=0;i<4;i++) {
		c_pp[i]=data->move[i]->pp;
	}
};

Trainer::Trainer() {
	name = "Champion Cynthia";
	battleSpritePath = "res/battleassets/opponentSprite1.png";
	srand(time(0));
    for (int i=0;i<3;i++) {
        party[i]=rand()%psize;
    }
}