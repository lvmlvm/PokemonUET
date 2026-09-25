#pragma once

#include <string>

using namespace std;

class Move {
	public:
		string name;
		int type;
		int power;
		int pp;
};

class PokemonData {
	public:
		string name;
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
};

class Trainer {
	public:
		string name;
		string battleSpritePath;
		Pokemon party[3];
		Trainer();
};

extern PokemonData pokemonData[];
extern int psize;