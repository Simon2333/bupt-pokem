#pragma once

#include "define.h"
#include"split.h"
#include <thread>
#include <mutex>
#include <string>
#include <condition_variable>
#include "pokemon.h"

// about socket
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

// about database
#include "sqlite3.h"
#pragma comment(lib, "sqlite3.lib")

using namespace std;
class Hub;

class Endpoint
{
	Hub& hub;

	//database
	sqlite3*& dataBase;

	//network 
	int port;
	string ip;
	SOCKET endpointSocket;
	SOCKET connSocket;
	volatile bool running;
	volatile bool online;
	char buf[BUF_LENGTH];

	//player
	int playerID;
	string playerUserName;
	bool isDuel;

	//thread
	mutex mtx;
	condition_variable cv;
	volatile bool timing;

	//two thread
	void timer();
	void listenFunc();

	//interface
	void ResetPassword(const string& oldPassword, const string& newPassword);
	void GetPlayerList();
	void GetPokemonList(int playerID);
	void GetPokemonByID(int pokemonID);
	void PokemonChangeName(const string& pokemonID, const string& newName);
	Pokemon *MakePokemonByRace(int atk, int hp, int def, int speed, int level, int exp, int race);
	void battle(int pokemonID, bool autoFight = false);
	void UseSkill(int skillID);
	void GetDuelStatistic();
	void battle(const string& pokemonID, const string& enemyID);
	void battle(const string& pokemonID, int enemyRaceID, int enemyLV);
	void ChooseBet();
	void Discard(const string& pokemonID);
	// other functions
	void SavePokemonToDB(const Pokemon& p,int id);
	void MakeRandomPokemon();

public:
	Endpoint(int playerID, sqlite3*& dataBase, Hub& hub);
	~Endpoint();

	//start endpoint
	int start();
	void process();

	//return user's information
	bool IsOnline() const { return online; }
	int GetPlayerID() const { return playerID; }
	string GetPlayerName() const { return playerUserName; }
	int GetPort()const { return port; }
};