#define _WINSOCK_DEPRECATED_NO_WARNINGS // ignore inet_ntoa errors
#define _CRT_SECURE_NO_WARNINGS					// ignore strcpy errors

#include "server.h"
#include <conio.h>
#include <thread>
#include <iostream>
#include <map>
#include<cstring>
#include"endpoint.h"
using namespace std;
//return now hub
Hub& Hub::GetInstance()
{
	static Hub result; // singleton
	return result;
}
//destruct hub
Hub::~Hub()
{
#pragma region delete endpoints
	// all endpoints should be destroyed in Hub::start()
	// add these lines here just in case :)
	mtx.lock();
	while (endpoints.size())
	{
		auto p = endpoints.back();
		delete p;
		endpoints.pop_back();
	}
	mtx.unlock();
#pragma endregion

	closesocket(hubSocket); // if socket has been closed, return WSAENOTSOCK, but that's ok
	closesocket(connSocket);

	/**
	 * WSACleanup, stop socket DLL
	 * - return 0 if WSACleanup has NOT been called(succeed)
	 * - return -1 if WSACleanup has been called
	*/
	while (WSACleanup() != -1)
		;
}
//start hub
void Hub::Start()
{
#pragma region init database
	cout << "Hub::Init database...";
	if (sqlite3_open("server.dataBase", &dataBase))
	{
		cout << endl << "Hub: Can NOT open database: " << sqlite3_errmsg(dataBase) << endl;
		return;
	}
	//create user info
	char* errMsg;
	string sql = "create table User(";
	sql += "id integer primary key,";
	sql += "name text unique not null,";
	sql += "password text not null,";
	sql += "win int not null,";
	sql += "total int not null";
	sql += ");";

	if (sqlite3_exec(dataBase, sql.c_str(), NonUseCallback, NULL, &errMsg) != SQLITE_OK)
	{
		sqlite3_free(errMsg);
	}
	//create pokemon info
	sql = "create table Pokemon(";
	sql += "id integer primary key,";
	sql += "userid integer not null,";
	sql += "name text not null,";
	sql += "race int not null,";
	sql += "type int not null,";
	sql += "element int not null,";
	sql += "atk int not null,";
	sql += "def int not null,";
	sql += "maxHp int not null,";
	sql += "speed int not null,";
	sql += "lv int not null,";
	sql += "exp int not null";
	sql += ");";
	if (sqlite3_exec(dataBase, sql.c_str(), NonUseCallback, NULL, &errMsg) != SQLITE_OK)
	{
		sqlite3_free(errMsg);
	}
	cout << "Done.\n";
#pragma endregion
#pragma region init socket
	// init socket DLL
	cout << "Hub: Init socket DLL...";
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata))
	{
		cout << endl << "Hub: Init network protocol failed." << endl;
		// system("pause");
		return;
	}
	cout << "Done." << endl;

	cout << "Hub: Init hub socket...";
	hubSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (hubSocket == INVALID_SOCKET)
	{
		cout << endl << "Hub: Init socket failed." << endl;
		closesocket(hubSocket);
		WSACleanup();
		// system("pause");
		return;
	}
	cout << "Done." << endl;

	// construct an address, including protocol & IP address & port
	sockaddr_in hubAddr;
	hubAddr.sin_family = AF_INET;
	hubAddr.sin_port = htons(HUB_PORT);
	hubAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY); // any ip address

	// bind socket to an address
	cout << "Hub: Socket binding...";
	// namespace conflicts: thread::bind and global::bind
	if (::bind(hubSocket, (sockaddr*)&hubAddr, sizeof(hubAddr)) == SOCKET_ERROR)
	{
		cout << endl << "Hub: Socket bind failed." << endl;
		closesocket(hubSocket);
		WSACleanup();
		// system("pause");
		return;
	}
	cout << "Done." << endl;

	// if request queue is full, client will get error: WSAECONNREFUSED
	cout << "Hub: Socket listen...";
	if (listen(hubSocket, REQ_QUEUE_LENGTH) == SOCKET_ERROR)
	{
		cout << WSAGetLastError();
		cout << endl << "Hub: Socket listen failed." << endl;
		closesocket(hubSocket);
		WSACleanup();
		// system("pause");
		return;
	}
	cout << "Done." << endl;
#pragma endregion
	// now listen successfully
	cout << endl << "Hub: Hub is running at " << HUB_PORT << endl;
	cout << "Press any key to stop hub.\n\n";

	// init thread
	running = true;
	thread listenThread(&Hub::ListenFunc, this);
	thread terminateThread(&Hub::TerminateFunc, this);
	listenThread.join();
	terminateThread.join();

	// destroy all endpoints
	mtx.lock();
	while (endpoints.size())
	{
		auto p = endpoints.back();
		delete p;
		endpoints.pop_back();
	}
	mtx.unlock();

	closesocket(hubSocket);
	WSACleanup();

	sqlite3_close(dataBase);

	cout << endl << "Hub: Hub stoped." << endl;
}
//listen while running
void Hub::ListenFunc()
{
	while (running)
	{
		sockaddr_in clientAddr; // client address
		int clientAddrLength = sizeof(clientAddr);
		connSocket = accept(hubSocket, (sockaddr*)&clientAddr, &clientAddrLength);
		if (connSocket == INVALID_SOCKET)
		{
			if (running)
			{
				// if not running, this thread must be terminated by terminateFunc
				// in that case the string below is not needed
				cout << "Hub: Link to client failed." << endl;
			}
			closesocket(connSocket);
			break;
		}
		// link successfully
		cout << "Hub: " << inet_ntoa(clientAddr.sin_addr) << " connected." << endl;
		/*
		 * process data
		 * format:
		 * - "login\n<username>\n<password>"
		 * - "signup\n<username>\n<password>"
		*/
		recv(connSocket, buf, BUF_LENGTH, 0);
		auto strs = split(buf);
		if (strs.size() == 1 && strs[0].length() == 0)
		{
			// blank request, maybe client is closed
		}
		else if (strs.size() < 3)
		{
			//error
			cout << "Hub: Invalid request: " << buf << endl;
			strcpy(buf, "Reject: Invalid request.\n");
			send(connSocket, buf, BUF_LENGTH, 0);
		}
		else if (strs[0] == "login")
			Login(strs[1], strs[2]);
		else if (strs[0] == "logon")
			Signup(strs[1], strs[2]);
		else
		{
			cout << "Hub: Invalid request: " << buf << endl;
			strcpy(buf, "Reject: Invalid request.\n");
			send(connSocket, buf, BUF_LENGTH, 0);
		}
		closesocket(connSocket);
	}
}
//press any key to stop
void Hub::TerminateFunc()
{
	// press any key to stop server
	_getch();
	running = false;
	closesocket(hubSocket); // stop listenning
}
//登录
void Hub::Login(const string& username, const string& password)
{
	//check valid of username and password
	if (ValidDetect(username, USERNAME) && ValidDetect(password, PASSWORD))
	{
		char** sqlResult;
		int nRow;
		int nColumn;
		char* errMsg;
		string sql = "SELECT id FROM User WHERE name = '" + username + "' AND password = '" + password + "'";
		if (sqlite3_get_table(dataBase, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK)
		{
			cout << "Hub: Sqlite3 error: " << errMsg << endl;
			strcpy(buf, "服务器数据库错误");
			sqlite3_free(errMsg);
		}
		else
		{
			if (nRow == 0)
			{
				// username and password mismatch
				cout << "Hub: Login: username '" << username << "' and password '" << password << "' mismatch.\n";
				strcpy(buf, "wrong username or password");
			}
			else
			{
				// username exist
				// check user state
				bool userExist = false;
				mtx.lock();
				int id = atoi(sqlResult[1]); // sqlResult[0] == "id", sqlResult[1] == playerID
				for (auto endpoint : endpoints)
				{
					if (endpoint->GetPlayerID() == id)
					{
						userExist = true;
						if (endpoint->IsOnline())
						{
							strcpy(buf, "用户已在其他设备登录");
						}
						else
						{
							// not online, return port
							strcpy(buf, to_string(endpoint->GetPort()).c_str());
						}
						break;
					}
				}
				mtx.unlock();
				if (!userExist) // add an endpoint
				{
					auto p = new Endpoint(id, dataBase, *this);
					int endpointPort = p->start();
					if (endpointPort == 0) // start ERROR, remove and delete this new endpoint
					{
						delete p;
						strcpy(buf, "服务器错误");
					}
					else // start normally, add this endpoint to endpoints
					{
						lock_guard<mutex> lock(mtx);
						endpoints.push_back(p);
						strcpy(buf, to_string(endpointPort).c_str());
						thread th(&Hub::Mornitor, this, p);
						th.detach();
					}
				}
			}
		}
	}
	send(connSocket, buf, BUF_LENGTH, 0);
}
//注册
void Hub::Signup(const string& username, const string& password)
{
	//check valid
	if (ValidDetect(username, USERNAME) && ValidDetect(password, PASSWORD))
	{
		char** sqlResult;
		int nRow;
		int nColumn;
		char* errMsg;
		string sql = "SELECT name FROM User WHERE name = '" + username + "'";
		if (sqlite3_get_table(dataBase, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK)
		{
			cout << "Hub: Sqlite3 error: " << errMsg << endl;
			strcpy(buf, "服务器数据库错误");
			sqlite3_free(errMsg);
		}
		else
		{
			if (nRow == 0)
			{
				// username NOT exist, add this user
				/*string sql = "INSERT INTO User(name, password) VALUES('" + username + "', '" + password + "');";*/
				string sql = "INSERT INTO User(name, password, win, total) VALUES('" + username + "', '" + password + "', 0, 0);";
				char* errMsg;
				if (sqlite3_exec(dataBase, sql.c_str(), NonUseCallback, NULL, &errMsg) != SQLITE_OK)
				{
					cout << "Hub: Sqlite3 error: " << errMsg << endl;
					sqlite3_free(errMsg);
					strcpy(buf, "服务器数据库错误");
				}
				else
				{
					cout << "Hub: Add user: " << username << " password: " << password << endl;
					strcpy(buf, "Accept.\n");
				}
				cout << sql << endl;
			}
			else
			{
				// username already exist
				cout << "Hub: Logon: username '" << username << "' already exist.\n";
				strcpy(buf, "用户名已存在");
			}
			sqlite3_free_table(sqlResult);
		}
	}
	send(connSocket, buf, BUF_LENGTH, 0);
}
//judge whether valid 
bool Hub::ValidDetect(const string & str, int op)
{
	if (op == USERNAME)
	{
		for (auto c : str)
		{
			if (c == '\b' || c == '\n' || c == '\t')
			{
				// contains \b \n \t
				cout << "Hub: Got an invalid username: " << str << endl;
				strcpy(buf, "不合法的用户名");
				return false;
			}
		}
		return true;
	}
	else if (op == PASSWORD)
	{
		for (auto c : str)
		{
			if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_'))
			{
				//not a letter or a digit or '_'
				cout << "Hub: Got an invalid password: " << str << endl;
				strcpy(buf, "不合法的密码");
				return false;
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}
//delete endpoint
void Hub::Mornitor(Endpoint* const endpoint)
{
	endpoint->process();

	// now endpoint reaches end
	mtx.lock();
	// remove from endpoints
	for (int i = 0; i < endpoints.size(); ++i)
	{
		if (endpoints[i] == endpoint)
		{
			endpoints.erase(endpoints.begin() + i);
			// if endpoints doesn't contain endpoint, that means endpoint has been deleted in ~Hub()
			delete endpoint;
			break;
		}
	}
	mtx.unlock();
}
/**get all user's information
 * format:
 * <userID> <userName> <online:0|1>
 */
string Hub::GetAllUser()
{
	struct temp
	{
		string name;
		bool online;
		string win;
		string total;
	};
	// get all user
	char** sqlResult;
	int nRow;
	int nColumn;
	char* errMsg;
	string sql = "SELECT id, name, win, total FROM User;";
	if (sqlite3_get_table(dataBase, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK)
	{
		cout << "Hub: Sqlite3 error: " << errMsg << endl;
		sqlite3_free(errMsg);
	}
	// construct playerMap
	map<int, temp> playerMap;
	
	for (int i = 0; i < nRow; ++i)
	{
		temp t = { sqlResult[4 * (i + 1) + 1], false, sqlResult[4 * (i + 1) + 2], sqlResult[4 * (i + 1) + 3] };
		playerMap.insert(make_pair(stoi(sqlResult[4 * (i + 1)]), t));
	}

	sqlite3_free_table(sqlResult);

	// judge user online and put online user first
	string result;
	mtx.lock();
	for (auto endpoint : endpoints)
	{
		playerMap[endpoint->GetPlayerID()].online = true;
	}
	mtx.unlock();

	for (auto& player : playerMap)
	{
		
		if (player.second.online)
		{
			// result = onlinePlayer + result;
			result = to_string(player.first) + ' ' + player.second.name + " 1 " + player.second.win + " " + player.second.total + "\n" + result;
		}
		else
		{
			// result = result + onlinePlayer
			result += to_string(player.first) + ' ' + player.second.name + " 0 " + player.second.win + " " + player.second.total + "\n";
		}
		cout << result << endl;
	}

	return result;
}