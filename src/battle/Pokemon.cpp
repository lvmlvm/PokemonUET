#include "battle/Pokemon.h"
#include "core/Rng.h"

Pokemon::Pokemon(int i) {
	data=pokemonData+i;
	c_hp=data->hp;
	for (int i=0;i<4;i++) {
		c_pp[i]=data->move[i]->pp;
	}
}

Pokemon::Pokemon() : Pokemon(0) {}

int Pokemon::species() const {
	return static_cast<int>(data - pokemonData);
}

Trainer::Trainer() {
	name = "Champion Cynthia";
	battleSpritePath = "battleassets/opponentSprites/opponentSprite1.png";
    for (int i=0;i<3;i++) {
        party[i]=randomInt(1, psize-1);
    }
}
