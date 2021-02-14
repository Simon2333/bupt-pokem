#pragma once
#include<string>
#include<vector>
#include<mutex>
#include"define.h"

#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")

#include"sqlite3.h"
#pragma comment(lib,"sqlite3.lib")

#include"endpoint.h"
#include"split.h"
using namespace std;

class Hub
{
private:
	//network
	SOCKET hubSocket;
	SOCKET connSocket;
	volatile bool running;
	char buf[BUF_LENGTH];

	//database
	sqlite3* dataBase;

	//a list of endpoints
	vector<Endpoint*>endpoints;

	mutex mtx;

	//logon and signup
	void Login(const string& username, const string& password);
	void Signup(const string& username, const string& password);

	//check valid of username and password
	bool ValidDetect(const string& str, int op);

	//thread
	void ListenFunc();
	void TerminateFunc();
	void Mornitor(Endpoint* const endpoint);

	//singleton
	//to makesure there's only one hub running
	Hub() {};
	Hub(Hub const&) = delete;
	Hub(Hub&&) = delete;
	Hub& operator=(Hub const&) = delete;
	~Hub();
	
public:
	static Hub& GetInstance();

	//init database and socket
	void Start();
	//get player list
	string GetAllUser();
};
inline int NonUseCallback(void* notUsed, int argc, char** argv, char** azColName) { return 0; }