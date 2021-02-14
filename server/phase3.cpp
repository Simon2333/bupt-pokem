#include "server.h"
#include <cstdlib>
#include <ctime>
#include "endpoint.h"
using namespace std;

int main()
{
	srand(time(NULL));
	Hub& hub = Hub::GetInstance();
	hub.Start();
	system("pause");
}