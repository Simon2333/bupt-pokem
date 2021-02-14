#pragma once
#include"pokemon.h"
#include<string>
#include"define.h"

// about socket
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

class BattleControl
{
private:
	Pokemon& p1, & p2;
	int timer1;
	int timer2;
	char buf[BUF_LENGTH];
	SOCKET& connSocket;
	string msg;
public:
	//simulator of battle
	BattleControl(Pokemon& pokemon1, Pokemon& pokemon2, SOCKET& connSocket) :p1(pokemon1), p2(pokemon2), connSocket(connSocket) {}
	bool start();
};