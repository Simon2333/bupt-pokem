#define _CRT_SECURE_NO_WARNINGS
#include"battlecontrol.h"
#include "define.h"
#include "server.h"
#include"pokemon.h"
#include<ctime>
#include<cstdlib>
#include<Windows.h>
#include <windows.h>
bool BattleControl::start()
{
	cout << p1.GetName() << " VS " << p2.GetName() << endl;

	//initialize two pokemon and timer
	//p1 = user
	//p2 = computer
	p1.RestoreAll();
	p2.RestoreAll();

	timer1 = 0;
	timer2 = 0;
	
	while (1)
	{
		if (recv(connSocket, buf, BUF_LENGTH, 0) == SOCKET_ERROR) return false; // get done
		cout << "--------------------------------------------" << endl;
		cout << timer1 << " " << timer2 << endl;
		while (timer1 < BATTLE_TIME && timer2 < BATTLE_TIME)
		{
			//each turn add interval
			timer1 += p1.GetCurInte();
			timer2 += p2.GetCurInte();
			p1.ChangeEner(3);
			p2.ChangeEner(3);
			cout << timer1 << " " << timer2 << endl;
		}
		int index;
		if (timer1 >= BATTLE_TIME && timer2 >= BATTLE_TIME)//both attack judge interval
		{
			timer1 -= BATTLE_TIME;
			timer2 -= BATTLE_TIME;
			if (p1.GetCurInte() >= p2.GetCurInte())
			{
				#pragma region p1 attack
				send(connSocket, "turn", BUF_LENGTH, 0);
				if (recv(connSocket, buf, BUF_LENGTH, 0) == SOCKET_ERROR) return false; // get player pokemon skill index
				sscanf(buf, "%d", &index);
				cout << buf <<"1>2"<< endl;
				msg = "1 ";
				if (p1.attack(p2, msg, index, false)) // player manual fight
					break;
				msg += to_string(p1.GetCurEner());
				msg += " ";
				strcpy(buf, msg.c_str());
				Sleep(500);
				send(connSocket, buf, BUF_LENGTH, 0);
				cout << buf << endl;
				if (recv(connSocket, buf, BUF_LENGTH, 0) == SOCKET_ERROR) return false; // get done
				#pragma endregion
				#pragma region p2 attack
				msg = "0 ";
				if (p2.attack(p1, msg, 0, true)) // enemy auto fight
					break;
				msg += to_string(p1.GetCurEner());
				msg += " ";
				strcpy(buf, msg.c_str());
				Sleep(2000);
				send(connSocket, buf, BUF_LENGTH, 0);
				cout << buf << endl;
				#pragma endregion
			}
			else
			{			
				#pragma region p2 attack
				msg = "0 ";
				if (p2.attack(p1, msg, 0, true))
					break;
				msg += to_string(p1.GetCurEner());
				msg += " ";
				strcpy(buf, msg.c_str());
				Sleep(2000);
				send(connSocket, buf, BUF_LENGTH, 0);
				cout << buf << endl;
				if (recv(connSocket, buf, BUF_LENGTH, 0) == SOCKET_ERROR) return false;// get done
				#pragma endregion
				#pragma region p1 attack
				send(connSocket, "turn", BUF_LENGTH, 0);
				if (recv(connSocket, buf, BUF_LENGTH, 0) == SOCKET_ERROR) return false;
				cout << buf <<"1<2"<< endl;
				sscanf(buf, "%d", &index);
				msg = "1 ";
				if (p1.attack(p2, msg, index, false))
					break;
				msg += to_string(p1.GetCurEner());
				msg += " ";
				strcpy(buf, msg.c_str());
				Sleep(500);
				send(connSocket, buf, BUF_LENGTH, 0);
				cout << buf << endl;
				#pragma endregion
			}
		}
		else if (timer1 >= BATTLE_TIME)// p1 attack
		{
			timer1 -= BATTLE_TIME;
			send(connSocket, "turn", BUF_LENGTH, 0);
			
			if (recv(connSocket, buf, BUF_LENGTH, 0) == SOCKET_ERROR) return false;
			sscanf(buf, "%d", &index);
			msg = "1 ";
			if (p1.attack(p2, msg, index, false))
				break;
			msg += to_string(p1.GetCurEner());
			msg += " ";
			strcpy(buf, msg.c_str());
			Sleep(500);
			send(connSocket, buf, BUF_LENGTH, 0);
			cout << buf << endl;
		}
		else if (timer2 >= BATTLE_TIME)//p2 attack
		{
			timer2 -= BATTLE_TIME;
			//p2 attack
			msg = "0 ";
			if (p2.attack(p1, msg, 0, true))
				break;
			msg += to_string(p1.GetCurEner());
			msg += " ";
			strcpy(buf, msg.c_str());
			Sleep(2000);
			send(connSocket, buf, BUF_LENGTH, 0);
			cout << buf << endl;
		}
	}
	//send buf
	msg += to_string(p1.GetCurEner());
	msg += " ";
	strcpy(buf, msg.c_str());
	send(connSocket, buf, BUF_LENGTH, 0);
	cout << buf << endl;
	if (p1.GetCurHp())//p1 won
	{
		cout << p1.GetName() << " won!\n\n";
		return true;
	}
	else//p2 won
	{
		cout << p2.GetName() << " won!\n\n";
		return false;
	}
}