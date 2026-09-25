#pragma once

#include <string>

class Move {
	public:
		std::string name;
		int type;
		int power;
		int pp;
};

class PokemonData {
	public:
		std::string name;
		int type;
		int stype;
		int hp;
		int atk;
		int def;
		int speed;
		Move* move[4];
};

class Pokemon {
	public:
		PokemonData* data;
		int c_hp;
		int c_pp[4];
		Pokemon();
		Pokemon(int i);
		int species() const; // index into pokemonData
};

class Trainer {
	public:
		std::string name;
		std::string battleSpritePath;
		Pokemon party[3];
		Trainer();
};

// Game data tables (battle/PokemonData.cpp).
extern const std::string Type[];
extern Move moves[];
extern PokemonData pokemonData[];
extern const int psize;
extern const double typeEffectiveness[19][19];
