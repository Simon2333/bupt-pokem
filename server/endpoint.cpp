#define _CRT_SECURE_NO_WARNINGS // ignore strcpy errors

#include "endpoint.h"
#include <iostream>
#include "define.h"
#include "server.h"
#include"pokemon.h"
#include"battlecontrol.h"
using namespace std;
//construct
Endpoint::Endpoint(int playerID, sqlite3*& dataBase, Hub& hub) : playerID(playerID), dataBase(dataBase), hub(hub)
{
	port = 0;
	running = false;
}
//destruct
Endpoint::~Endpoint()
{
	running = false;
	while (timing)
	{
		unique_lock<mutex> lock(mtx);
		online = true;
		lock.unlock();
		cv.notify_one();
		lock.lock();
	}

	closesocket(endpointSocket);

	if (port)
		cout << "Endpoint[" << playerID << "]: Endpoint stoped at " << port << endl;
}
//start endpoint
int Endpoint::start()
{
	// get playerUsername
#pragma region get playerusername
	char** sqlResult;
	int nRow;
	int nColumn;
	char* errMsg;
	string sql = "SELECT name FROM User where id=" + to_string(playerID) + ";";
	if (sqlite3_get_table(dataBase, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK)
	{
		cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
		sqlite3_free(errMsg);
		return 0;
	}
	else if (nRow == 0)
	{
		cout << "Endpoint[" << playerID << "]: Database content error.\n";
		sqlite3_free_table(sqlResult);
		return 0;
	}
	else
	{
		playerUserName = sqlResult[1];
		sqlite3_free_table(sqlResult);
	}
#pragma endregion
#pragma region init socket
	/*
	 * init endpoint socket
	 *
	 * function: socket(int domain, int type, int protocol);
	 * domain: AF_INET or PF_INET
	 *   - AF for Address Family
	 *   - PF for Protocol Family
	 *   - in Windows, AF_INET == PF_INET
	 * type: SOCK_STREAM or SOCK_DGRAM or SOCK_RAW
	 * protocol: use IPPROTO_TCP for TCP/IP
	*/
	cout << "Endpoint[" << playerID << "]: init socket..." << endl;
	endpointSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (endpointSocket == INVALID_SOCKET)
	{
		cout << "Endpoint[" << playerID << "]: Init socket failed." << endl;
		closesocket(endpointSocket);
		// system("pause");
		return 0;
	}
	cout << "Done." << endl;


	// construct an address, including protocol & IP address & port
	sockaddr_in endpointAddr;
	endpointAddr.sin_family = AF_INET;
	endpointAddr.sin_port = htons(0);											 // port = 0 so windows will give us a free port
	endpointAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY); // any ip address


	// bind socket to an address
	cout << "Endpoint[" << playerID << "]: Socket binding..." << endl;
	if (::bind(endpointSocket, (sockaddr*)&endpointAddr, sizeof(endpointAddr)) == SOCKET_ERROR)
	{
		cout << "Endpoint[" << playerID << "]: Socket bind failed." << endl;
		closesocket(endpointSocket);
		// system("pause");
		return 0;
	}
	cout << "Done." << endl;

	// now we get a free port by OS
	int endpointAddrLength = sizeof(endpointAddr);
	getsockname(endpointSocket, (sockaddr*)&endpointAddr, &endpointAddrLength);
	port = ntohs(endpointAddr.sin_port);

	// if request queue is full, client will get error: WSAECONNREFUSED
	cout << "Endpoint[" << playerID << "]: Socket listen..." << endl;
	if (listen(endpointSocket, REQ_QUEUE_LENGTH) == SOCKET_ERROR)
	{
		cout << WSAGetLastError();
		cout << "Endpoint[" << playerID << "]: Socket listen failed." << endl;
		closesocket(endpointSocket);
		// system("pause");
		return 0;
	}
	cout << "Done." << endl;

	// now listen successfully
	cout << "Endpoint[" << playerID << "]: Endpoint is running at " << port << endl;
#pragma endregion
	running = true;
	return port;
}
//start thread
void Endpoint::process()
{
	while (running)
	{
		online = false;
		timing = true;

		thread timerThread(&Endpoint::timer, this);
		thread listenThread(&Endpoint::listenFunc, this);
		timerThread.join();
		listenThread.join();
	}
}
//thread:listen
void Endpoint::listenFunc()
{
	//client addr
	sockaddr_in clientAddr;
	int clientAddrLength = sizeof(clientAddr);
	connSocket = accept(endpointSocket, (sockaddr*)&clientAddr, &clientAddrLength);
	while (timing)
	{
		unique_lock<mutex> lock(mtx);
		online = true;
		lock.unlock();
		cv.notify_one();
		lock.lock();
	}
	if (connSocket == INVALID_SOCKET)
	{
		return;
	}
	int ret = recv(connSocket, buf, BUF_LENGTH, 0);
	
	/**
	 * recv(connSocket, buf, BUF_LENGTH, 0)
	 * - return bytes of buf
	 * - return 0 if client socket closed or get an empty line
	 * - return SOCKET_ERROR(-1) if server socket is closed or client unexpectedly terminated
	*/
	while (ret != 0 && ret != SOCKET_ERROR && running)
	{
		//split buf by " "(space) and get order
		auto strs = split(buf);
		if (strs[0] == "logout")
		{
			running = false;
		}
		else if (strs[0] == "getPlayerList")
		{
			GetPlayerList();
		}
		else if (strs[0] == "resetPassword" && strs.size() == 3)
		{
			ResetPassword(strs[1], strs[2]);
		}
		else if (strs[0] == "getPokemonList" && strs.size() < 3)
		{
			if (strs.size() == 2)
			{
				GetPokemonList(stoi(strs[1]));
			}
			else
			{
				GetPokemonList(playerID);
			}
		}
		else if (strs[0] == "getPokemon" && strs.size() == 2)
		{
			GetPokemonByID(stoi(strs[1]));
		}
		else if (strs[0] == "pokemonChangeName" && strs.size() == 3)
		{
			PokemonChangeName(strs[1], strs[2]);
		}
		else if (strs[0] == "getDuelStatistic")
		{
			GetDuelStatistic();
		}
		else if (strs[0] == "battle" && strs.size() == 4)
		{
			if (strs[1] == "DUEL")
				isDuel = true;
			else
				isDuel = false;
			battle(strs[2],strs[3]);
		}
		else if (strs[0] == "chooseBet" && strs.size() == 1)
		{
			ChooseBet();
		}
		else if (strs[0] == "discard" && strs.size() == 2)
		{
			Discard(strs[1]);
		}
		else
		{
			cout << "Endpoint[" << playerID << "]: Invalid request.\n";
			strcpy(buf, "Reject: Invalid request.\n");
			send(connSocket, buf, BUF_LENGTH, 0);
		}
		if (running)
		{
			ret = recv(connSocket, buf, BUF_LENGTH, 0);
		}
	}

	if (running)
	{
		if (ret == SOCKET_ERROR || ret == 0)
		{
			cout << "Endpoint[" << playerID << "]: Client unexpected offline, start timing." << endl;
		}
		else
		{
			// running == false, user logout
			cout << "Endpoint[" << playerID << "]: User logout." << endl;
		}
	}
	closesocket(connSocket);
}
//wait for player re-login for 10 minutes
void Endpoint::timer()
{
	using namespace std::chrono_literals;

	/**
	 * wait for player re-login for 10 minutes
	 *
	 * condition_variable::wait_for(lock, time, condition);
	 * - lock is for variables in this function
	 *   - now lock is for bool running
	 * - return false when time out and condition == false
	 * - return true when otherwise
	*/
	unique_lock<mutex> lock(mtx);
	if (!cv.wait_for(lock, 10min, [this] { return online; }))
	{
		// player is offline
		running = false;
		timing = false;
		closesocket(endpointSocket);
	}
	else
	{
		timing = false;
	}
}
//reset password
void Endpoint::ResetPassword(const string& oldPassword, const string& newPassword)
{
	// check oldPassword
	char** sqlResult;
	int nRow;
	int nColumn;
	char* errMsg;
	string sql;
	sql = "SELECT name FROM User where id=" + to_string(playerID) + " and password='" + oldPassword + "';";
	if (sqlite3_get_table(dataBase, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK)
	{

		cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
		sqlite3_free(errMsg);
		strcpy(buf, "服务器错误");
		send(connSocket, buf, BUF_LENGTH, 0);
		return;
	}
	else if (nRow == 0)
	{
		// wrong password
		sqlite3_free_table(sqlResult);
		strcpy(buf, "旧密码不正确");
		send(connSocket, buf, BUF_LENGTH, 0);
		return;
	}
	else
	{
		sqlite3_free_table(sqlResult);
	}

	// update password
	sql = "update User set password='" + newPassword + "' where id=" + to_string(playerID) + ";";
	if (sqlite3_get_table(dataBase, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK)
	{
		cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
		sqlite3_free(errMsg);
		strcpy(buf, "服务器错误");
		send(connSocket, buf, BUF_LENGTH, 0);
		return;
	}
	else
	{
		sqlite3_free_table(sqlResult);
		strcpy(buf, "Accept.\n");
		send(connSocket, buf, BUF_LENGTH, 0);
	}
	return;
}
//return playerlist
void Endpoint::GetPlayerList()
{
	//get list from hub
	strcpy(buf, hub.GetAllUser().c_str());
	send(connSocket, buf, BUF_LENGTH, 0);
}
//make a random pokemon
void Endpoint::MakeRandomPokemon()
{
	
	int raceIndex = rand() % RACE_NUM;
	switch (raceIndex)
	{
	case 0:
	{
		Squirtle pokem("Squirtle");
		SavePokemonToDB(pokem, -1);
		break;
	}
	case 1:
	{
		Charmander pokem("Charmander");
		SavePokemonToDB(pokem,-1);
		break;
	}
	case 2:
	{
		Bulbasaur pokem("Bulbasaur");
		SavePokemonToDB(pokem,-1);
		break;
	}
	default:
	{
		Squirtle pokem("Squirtle");
		SavePokemonToDB(pokem, -1);
		break;
	}
	}
}
//return pokemonlist by player id
void Endpoint::GetPokemonList(int playerID)
{
	//get pokemon information
	cout << "GetPokemonList " << playerID << endl;
	char** sqlResult;
	int nRow;
	int nColumn;
	char* errMsg;
	string sql;
	sql = "SELECT id, name, race, lv FROM Pokemon where userid=" + to_string(playerID) + ";";
	if (sqlite3_get_table(dataBase, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK)
	{
		cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
		sqlite3_free(errMsg);
		strcpy(buf, "服务器错误.\n");
		send(connSocket, buf, BUF_LENGTH, 0);
		return;
	}
	//get user information
	char** sqlResult2;
	int nRow2;
	int nColumn2;
	char* errMsg2;
	string sql2 = "SELECT id, name, win, total FROM User;";
	if (sqlite3_get_table(dataBase, sql2.c_str(), &sqlResult2, &nRow2, &nColumn2, &errMsg2) != SQLITE_OK)
	{
		cout << "Hub: Sqlite3 error: " << errMsg2 << endl;
		sqlite3_free(errMsg2);
	}

	string result2 = sqlResult2[7];
	if (nRow == 0 && playerID == this->playerID)
	{
		cout << "yougetnotpokemon";
		MakeRandomPokemon();
		GetPokemonList(playerID);
	}
	//first signup user
	else if (nRow < 3 && playerID == this->playerID && result2=="0")
	{
		// add pokemons for user till his pokemon number be 3
		for (int i = 0; i < 3 - nRow; ++i)
		{
			MakeRandomPokemon();
		}
		GetPokemonList(playerID);
	}
	else
	{
		string result;
		for (int i = 0; i < nRow; ++i)
		{
			result += sqlResult[4 * (i + 1)]; // id
			result += ' ';
			result += sqlResult[4 * (i + 1) + 1]; // name
			result += ' ';
			result += sqlResult[4 * (i + 1) + 2]; // race
			result += ' ';
			result += sqlResult[4 * (i + 1) + 3]; // lv
			result += '\n';
		}
		strcpy(buf, result.c_str());
		send(connSocket, buf, BUF_LENGTH, 0);
	}
	sqlite3_free_table(sqlResult);
}
//update or save new pokemon to database
void Endpoint::SavePokemonToDB(const Pokemon& p,int id)
{
	if (id == -1)//insert new pokemon
	{
		string sql = "INSERT INTO Pokemon(userid, name, race, type, element, atk, def, maxHp, speed, lv, exp) VALUES('";
		sql += to_string(playerID) + "','";
		sql += p.GetName() + "',";
		sql += to_string(p.GetRace()) + ",";
		sql += to_string(p.GetType()) + ",";
		sql += to_string(p.GetElement()) + ",";
		sql += to_string(p.GetBaseAtk()) + ",";
		sql += to_string(p.GetBaseDef()) + ",";
		sql += to_string(p.GetBaseHp()) + ",";
		sql += to_string(p.GetBaseInterval()) + ",";
		sql += to_string(p.GetLevel()) + ",";
		sql += to_string(p.GetExp()) + ");";
		cout << sql;
		char* errMsg;
		if (sqlite3_exec(dataBase, sql.c_str(), NonUseCallback, NULL, &errMsg) != SQLITE_OK)
		{
			cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
			sqlite3_free(errMsg);
		}
	}
	else//update old pokemon
	{
		string sql = "update Pokemon set atk=";
		sql += to_string(p.GetBaseAtk()) + ", def=";
		sql += to_string(p.GetBaseDef()) + ", maxHp=";
		sql += to_string(p.GetBaseHp()) + ", speed=";
		sql += to_string(p.GetBaseInterval()) + ", lv=";
		sql += to_string(p.GetLevel()) + ", exp=";
		sql += to_string(p.GetExp());
		sql += " where id=";
		sql += to_string(id) + ";";
		char* errMsg;
		if (sqlite3_exec(dataBase, sql.c_str(), NonUseCallback, NULL, &errMsg) != SQLITE_OK)
		{
			cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
			sqlite3_free(errMsg);
		}
	}
}
//return pokemon information by pokemon id
void Endpoint::GetPokemonByID(int pokemonID)
{
	char** sqlResult;
	int nRow;
	int nColumn;
	char* errMsg;
	string sql;
	sql = "SELECT id, name, race, atk, def, maxHp, speed, lv, exp, type, element FROM Pokemon where id=" + to_string(pokemonID) + ";";
	if (sqlite3_get_table(dataBase, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK)
	{
		cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
		sqlite3_free(errMsg);
		strcpy(buf, "Reject: Server error.\n");
		send(connSocket, buf, BUF_LENGTH, 0);
		return;
	}
	string result;
	result += sqlResult[11 + 0]; // id
	result += ' ';
	result += sqlResult[11 + 1]; // name
	result += ' ';
	result += sqlResult[11 + 2]; // race
	result += ' ';
	result += sqlResult[11 + 3]; // atk
	result += ' ';
	result += sqlResult[11 + 4]; // def
	result += ' ';
	result += sqlResult[11 + 5]; // maxHp
	result += ' ';
	result += sqlResult[11 + 6]; // speed
	result += ' ';
	result += sqlResult[11 + 7]; // lv
	result += ' ';
	result += sqlResult[11 + 8]; // exp
	result += ' ';
	result += sqlResult[11 + 9]; // type
	result += ' ';
	result += sqlResult[11 + 10]; // element
	result += '\n';
	cout << sqlResult << endl;
	cout << result << endl;
	strcpy(buf, result.c_str());
	send(connSocket, buf, BUF_LENGTH, 0);
	sqlite3_free_table(sqlResult);
}
//change pokemon name
void Endpoint::PokemonChangeName(const string& pokemonID, const string& newName)
{
	string sql = "update Pokemon set name = '" + newName + "' where id=" + pokemonID + ";";
	cout << "change pokemon " << pokemonID << " to " << newName << endl;
	char* errMsg;
	if (sqlite3_exec(dataBase, sql.c_str(), NonUseCallback, NULL, &errMsg) != SQLITE_OK)
	{
		cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
		sqlite3_free(errMsg);
		strcpy(buf, errMsg);
		send(connSocket, buf, BUF_LENGTH, 0);
	}
	else
	{
		strcpy(buf, "Accept.\n");
		send(connSocket, buf, BUF_LENGTH, 0);
	}
}
//get duel information
void Endpoint::GetDuelStatistic()
{
	char** sqlResult;
	int nRow;
	int nColumn;
	char* errMsg;
	string sql;
	sql = "SELECT name, win, total FROM User where id=" + to_string(playerID) + ";";
	if (sqlite3_get_table(dataBase, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK)
	{
		cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
		sqlite3_free(errMsg);
		strcpy(buf, "Reject: Server error.\n");
		send(connSocket, buf, BUF_LENGTH, 0);
		return;
	}
	string result = sqlResult[3];
	result += ' ';
	result += sqlResult[4];
	result += ' ';
	result += sqlResult[5];
	strcpy(buf, result.c_str());
	send(connSocket, buf, BUF_LENGTH, 0);
	sqlite3_free_table(sqlResult);
}
//make a new pokemon by race
Pokemon* Endpoint::MakePokemonByRace(int atk,int hp,int def,int speed,int level,int exp,int race)
{
	switch (race)
	{
	case CHARMANDER:
	{
		Charmander *p = new Charmander(atk, hp, def, speed, level, exp, "charmander");
		return p;
	}
	case BULBASAUR:
	{
		Bulbasaur* p = new Bulbasaur(atk, hp, def, speed, level, exp, "bulbasaur");
		/*Bulbasaur *p = new Bulbasaur("charmander");*/
		return p;
	}
	case SQUIRTLE:
	{
		Squirtle* p = new Squirtle(atk, hp, def, speed, level, exp, "squirtle");
		/*Squirtle *p = new Squirtle("charmander");*/
		return p;
	}
	}
}
//start battle 
void Endpoint::battle(const string& pokemonID, const string& enemyID)
{
	cout << pokemonID << " " << enemyID << " is gonna fight" << endl;
#pragma region get p1 info
	char** sqlResult;
	int nRow;
	int nColumn;
	char* errMsg;
	string sql;
	sql = "SELECT id, name, race, atk, def, maxHp, speed, lv, exp, type, element FROM Pokemon where id=" + pokemonID + ";";
	if (sqlite3_get_table(dataBase, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK)
	{
		cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
		sqlite3_free(errMsg);
		strcpy(buf, "Reject: Server error.\n");
		send(connSocket, buf, BUF_LENGTH, 0);
		return;
	}
	int currentPokemonID = stoi(sqlResult[11 + 0]);
	int race = stoi(sqlResult[11 + 2]);
	string result;
	result += sqlResult[11 + 0];
	result += ' ';//id
	result += sqlResult[11 + 1] ;//name
	result += ' ';
	result += sqlResult[11 + 2] ;//race
	result += ' ';
	result += sqlResult[11 + 3] ;//atk
	result += ' ';
	result += sqlResult[11 + 5] ;//hp
	result += '\n';
	Pokemon* p1 = MakePokemonByRace(stoi(sqlResult[11 + 3]),
		stoi(sqlResult[11 + 5]), stoi(sqlResult[11 + 4]), stoi(sqlResult[11 + 6]),
		stoi(sqlResult[11 + 7]), stoi(sqlResult[11 + 8]), stoi(sqlResult[11 + 2]));
	for (int i = 0; i < 4; ++i)
	{
		result += p1->GetSkillName(i) + '\n';
		result += p1->GetSkillDisc(i) + '\n';
	}
#pragma endregion
#pragma region get p2 info
	char** sqlResult2;
	int nRow2;
	int nColumn2;
	char* errMsg2;
	string sql2;
	sql2 = "SELECT id, name, race, atk, def, maxHp, speed, lv, exp, type, element FROM Pokemon where id=" + enemyID + ";";
	if (sqlite3_get_table(dataBase, sql2.c_str(), &sqlResult2, &nRow2, &nColumn2, &errMsg2) != SQLITE_OK)
	{
		cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg2 << endl;
		sqlite3_free(errMsg);
		strcpy(buf, "Reject: Server error.\n");
		send(connSocket, buf, BUF_LENGTH, 0);
		return;
	}
	int race2 = stoi(sqlResult2[11 + 2]);
	result += sqlResult2[11 + 0];
	result += ' ';//id
	result += sqlResult2[11 + 1];//name
	result += ' ';
	result += sqlResult2[11 + 2];//race
	result += ' ';
	result += sqlResult2[11 + 3];//atk
	result += ' ';
	result += sqlResult2[11 + 5];//hp
	result += '\n';
	Pokemon* p2 = MakePokemonByRace(stoi(sqlResult2[11 + 3]),
		stoi(sqlResult2[11 + 5]), stoi(sqlResult2[11 + 4]), stoi(sqlResult2[11 + 6]),
		stoi(sqlResult2[11 + 7]), stoi(sqlResult2[11 + 8]), stoi(sqlResult2[11 + 2]));
#pragma endregion
	strcpy(buf, result.c_str());
	send(connSocket, buf, BUF_LENGTH, 0);
	// construct battle controller

	BattleControl battle = BattleControl(*p1, *p2,connSocket);
	char** sqlResult3;
	int nRow3;
	int nColumn3;
	char* errMsg3;
	string sql3 = "select win, total from User where id=" + to_string(playerID) + ";";
	if (sqlite3_get_table(dataBase, sql3.c_str(), &sqlResult3, &nRow3, &nColumn3, &errMsg3) != SQLITE_OK)
	{
		cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg2 << endl;
		sqlite3_free(errMsg2);
		strcpy(buf, "Reject: Server error.\n");
		send(connSocket, buf, BUF_LENGTH, 0);
		return;
	}

	if (battle.start())
	{
		// win
		if (isDuel)//决斗赛
		{
			// change win rate
			string sql4 = "update User set win=" + to_string(stoi(sqlResult3[2]) + 1) + ", total=" + to_string(stoi(sqlResult3[3]) + 1) + " where id=" + to_string(playerID) + ";";
			char* errMsg4;
			if (sqlite3_exec(dataBase, sql4.c_str(), NonUseCallback, NULL, &errMsg4) != SQLITE_OK)
			{
				cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg4 << endl;
				sqlite3_free(errMsg4);
			}

			// get a new pokemon
			SavePokemonToDB(*p2, -1);
		}

		// p1 gain exp
		p1->GainExp(50);

	}
	else if(isDuel)
	{
		//lose
		// change win rate
		string sql4 = "update User set total=" + to_string(stoi(sqlResult3[3]) + 1) + " where id=" + to_string(playerID) + ";";
		char* errMsg4;
		if (sqlite3_exec(dataBase, sql4.c_str(), NonUseCallback, NULL, &errMsg4) != SQLITE_OK)
		{
			cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg4 << endl;
			sqlite3_free(errMsg4);
		}
	}
	sqlite3_free_table(sqlResult3);

	SavePokemonToDB(*p1, currentPokemonID);
	delete p2;
	sqlite3_free_table(sqlResult2);
	sqlite3_free_table(sqlResult);
}
//choose bet to discard
void Endpoint::ChooseBet()
{
	// get all pokemon id
	char** sqlResult;
	int nRow;
	int nColumn;
	char* errMsg;
	string sql;
	sql = "SELECT id FROM Pokemon where userid=" + to_string(playerID) + ";";
	if (sqlite3_get_table(dataBase, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK)
	{
		cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
		sqlite3_free(errMsg);
		strcpy(buf, "Reject: Server error.\n");
		send(connSocket, buf, BUF_LENGTH, 0);
		return;
	}

	vector<string> ids;
	for (int i = 0; i < nRow; ++i)
	{
		ids.push_back(sqlResult[i + 1]);
	}

	// get 3 random id
	int id[3];
	int index[3];
	for (int i = 0; i < 3; ++i)
	{
		index[i] = rand() % ids.size();
		id[i] = stoi(ids[index[i]]);
		ids.erase(ids.begin() + index[i]);
	}

	sqlite3_free_table(sqlResult);

	// get 3 pokemon details
	for (int i = 0; i < 3; ++i)
	{
		GetPokemonByID(id[i]);
		recv(connSocket, buf, BUF_LENGTH, 0); // read done
	}
}
//discard pokemon
void Endpoint::Discard(const string& pokemonID)
{
	string sql = "delete from Pokemon where id=";
	sql += pokemonID + ";";
	char* errMsg;
	if (sqlite3_exec(dataBase, sql.c_str(), NonUseCallback, NULL, &errMsg) != SQLITE_OK)
	{
		cout << "Endpoint[" << playerID << "]: Sqlite3 error: " << errMsg << endl;
		sqlite3_free(errMsg);
	}
	cout << sql << endl;
}