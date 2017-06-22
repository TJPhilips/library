// BattleshipBot.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <winsock2.h>
#include <math.h>
#include <cmath>
#pragma comment(lib, "wsock32.lib")

//#define STUDENT_NUMBER		"14021345"
#define STUDENT_NUMBER		"Get rekt m8" //name on server
#define STUDENT_FIRSTNAME	"Toby"
#define STUDENT_FAMILYNAME	"Philips"

#define IP_ADDRESS_SERVER	"127.0.0.1" //main server
#define IP_ADDRESS_SERVER "164.11.80.27" //server to see how many hits taken and given

#define PORT_SEND	 1924 // We define a port that we are going to use.
#define PORT_RECEIVE 1925 // We define a port that we are going to use.


#define MAX_BUFFER_SIZE	500
#define MAX_SHIPS		200

#define FIRING_RANGE	100

#define MOVE_LEFT		-1
#define MOVE_RIGHT		 1
#define MOVE_UP			 1
#define MOVE_DOWN		-1
#define MOVE_FAST		 2
#define MOVE_SLOW		 1


SOCKADDR_IN sendto_addr;
SOCKADDR_IN receive_addr;

SOCKET sock_send;  // This is our socket, it is the handle to the IO address to read/write packets
SOCKET sock_recv;  // This is our socket, it is the handle to the IO address to read/write packets

WSADATA data;

char InputBuffer [MAX_BUFFER_SIZE];



int myX;
int myY;
int myHealth;
int myFlag;

int number_of_ships;
int shipX[MAX_SHIPS];
int shipY[MAX_SHIPS];
int shipHealth[MAX_SHIPS];
int shipFlag[MAX_SHIPS];

bool fire = false;
int fireX;
int fireY;

bool moveShip = false;
int moveX;
int moveY;

bool setFlag = true;
int new_flag = 271195;


void fire_at_ship(int X, int Y);
void move_in_direction(int left_right, int up_down);
void set_new_flag(int newFlag);



/*************************************************************/
/********* Your tactics code starts here *********************/
/*************************************************************/

int up_down = MOVE_LEFT*MOVE_SLOW;
int left_right = MOVE_UP*MOVE_FAST;

int shipDistance[MAX_SHIPS];

int number_of_friends;
int friendX[MAX_SHIPS];
int friendY[MAX_SHIPS];
int friendHealth[MAX_SHIPS];
int friendFlag[MAX_SHIPS];
int friendDistance[MAX_SHIPS];

int number_of_enemies;
int enemyX[MAX_SHIPS];
int enemyY[MAX_SHIPS];
int enemyHealth[MAX_SHIPS];
int enemyFlag[MAX_SHIPS];
int enemyDistance[MAX_SHIPS];

int tactic = 1;
bool following_friend;
int currentlyFollowingID = 9999; //Default set high
int myFlagID = 2795;

int myOffset = 5;

int IsaFriend(int index)
{
	int rc;

	rc = 0;

	int id, xpart, ypart;
	id = shipFlag[index] / 1000000;
	if (id == 0)
	{
		id = 1;
	}

	xpart = (shipFlag[index] / 1000) % id;
	ypart = shipFlag[index] % (id * 1000 + xpart);

	//printf("Firends ID = %d\n", id);
	if (((1330 <= id) && (id < 1338) && (id != 1331)) && ((xpart + 10 > shipX[index]) && (xpart - 10 < shipX[index])) && ((ypart + 10 > shipY[index]) && (ypart - 10 < shipY[index])))
	{
		rc = id;  //Return friendly
	}

	return rc;
}

void headTowardsCoordinate(int gotoX, int gotoY, int speed){

	if (gotoX != -1)
	{
		if (myX < gotoX)
		{
			left_right = 1 * speed;
		}
		else
		{
			left_right = -1 * speed;
		}

		if (myY < gotoY)
		{
			up_down = 1 * speed;
		}
		else
		{
			up_down = -1 * speed;
		}
	}
}


//Figures out which enemy is closest if no ship is less than 200 away returns 0 so it doesn't shoot
int closetsEnemy()
{
	int nearestShip = 0;
	int currentDistance = 400, nearsetDistance = 400;
	if (number_of_enemies != 0)
	{
		for (int i = 0; i <= number_of_enemies; i++)
		{
			currentDistance = (int)sqrt(((double)(((myX - enemyX[i])*(myX - enemyX[i])) + ((myY - enemyY[i])*(myY - enemyY[i])))));
			if (currentDistance < nearsetDistance)
			{
				nearestShip = i;
				nearsetDistance = currentDistance;
			}
		}
	}
	else
	{
		nearestShip = -1;
	}

	return nearestShip;
}

int calculateDistanceBetweenTwoShips(int x1, int y1, int x2, int y2){
	return (int)(sqrtf(((x2 - x1)*(x2 - x1)) + ((y2 - y1)*(y2 - y1))));
}

int calculateClosest(int x[], int y[]) {
	int closestFriend = 0;
	int closestDistance = 10000000;
	for (int i = 0; i < 6; i++) {
		int currentDistance = calculateDistanceBetweenTwoShips(myX, myY, x[i], y[i]);
		if ((currentDistance < closestDistance) && (i != myOffset)) {
			closestDistance = currentDistance;
			closestFriend = i;
		}
	}
	return closestFriend;
}



void tactics1()
{
	following_friend = false;
	int i, enemyToShoot;
	set_new_flag(((((myFlagID)* 1000) + myX) * 1000) + myY);
	printf("%d\n", myFlag);

	//Bounces off the wall
	/*
	if ( myY >= 990)
	{
	up_down = MOVE_DOWN*MOVE_FAST;
	}

	if (myX <= 10)
	{
	left_right = MOVE_RIGHT*MOVE_FAST;
	}

	if ( myY <= 10)
	{
	up_down = MOVE_UP*MOVE_FAST;
	}

	if (myX >= 990)
	{
	left_right = MOVE_LEFT*MOVE_FAST;
	}*/

	number_of_friends = 0;
	number_of_enemies = 0;

	if (number_of_ships > 1)
	{
		for (i = 1; i<number_of_ships; i++)
		{
			int flagID = IsaFriend(i);
			if (flagID != 0)
			{
				friendX[number_of_friends] = shipX[i];
				friendY[number_of_friends] = shipY[i];
				friendHealth[number_of_friends] = shipHealth[i];
				friendFlag[number_of_friends] = shipFlag[i];
				friendDistance[number_of_friends] = shipDistance[i];
				number_of_friends++;

				if ((flagID < currentlyFollowingID) && (flagID < myFlagID)){
					currentlyFollowingID = flagID;
					headTowardsCoordinate(friendX[i], friendY[i], 2);
					following_friend = true;
				}


			}
			else
			{
				enemyX[number_of_enemies] = shipX[i];
				enemyY[number_of_enemies] = shipY[i];
				enemyHealth[number_of_enemies] = shipHealth[i];
				enemyFlag[number_of_enemies] = shipFlag[i];
				enemyDistance[number_of_enemies] = shipDistance[i];
				number_of_enemies++;
			}
		}


	}

	if (following_friend == false){

		if (number_of_friends > 1 && number_of_friends <= 3){
			//Group size 2 - 3
		}



	}


	enemyToShoot = closetsEnemy();

	// This part is used to move towards an enemy if one is near if not it moves in a basic formation
	if (enemyToShoot != -1)
	{
		//headTowardsCoordinate(enemyX[enemyToShoot], enemyY[enemyToShoot],2);
		fire_at_ship(enemyX[enemyToShoot], enemyY[enemyToShoot]);

	}

	move_in_direction(left_right, up_down);

}

/*
void tactics2(){
int i;
set_new_flag(((((1331)*1000)+myX)*1000)+myY);

if ( myY >= 990)
{
up_down = MOVE_DOWN*MOVE_FAST;
}

if (myX <= 10)
{
left_right = MOVE_RIGHT*MOVE_FAST;
}

if ( myY <= 10)
{
up_down = MOVE_UP*MOVE_FAST;
}

if (myX >= 990)
{
left_right = MOVE_LEFT*MOVE_FAST;
}

number_of_friends = 0;
number_of_enemies = 0;

if (number_of_ships > 1)
{
for (i=1; i<number_of_ships; i++)
{
int flagID = IsaFriend(i);
if (flagID != 0)
{
friendX[number_of_friends] = shipX[i];
friendY[number_of_friends] = shipY[i];
friendHealth[number_of_friends] = shipHealth[i];
friendFlag[number_of_friends] = shipFlag[i];
friendDistance[number_of_friends] = shipDistance[i];
number_of_friends++;

/*switch(flagID){
case 1330:
headTowardsCoordinate(friendX[i], friendY[i], 2);
following_friend = true;
break;
default:
following_friend = false;
break;
}


}
else
{
enemyX[number_of_enemies] = shipX[i];
enemyY[number_of_enemies] = shipY[i];
enemyHealth[number_of_enemies] = shipHealth[i];
enemyFlag[number_of_enemies] = shipFlag[i];
enemyDistance[number_of_enemies] = shipDistance[i];
number_of_enemies++;
}
}
}

move_in_direction(left_right, up_down);
}
*/

void tactics()
{
	switch (tactic) //Switches between tactics
	{
		//If I use tactic 1
	case 1:
		tactics1();

	default:
		break;
	}
}

void messageReceived(char* msg)
{

	printf("%s\n", msg);
	char newMessage[100];
	int X[6], Y[6];

	/*
	Jack - 0
	Ricky - 1
	Toby1 - 2
	toby2 - 3
	toby3 - 4
	Htoby3- 5
	*/
	//Reset Arrays
	for (int i = 0; i <= 5; i++){
		X[i] = 0;
		Y[i] = 0;
	}
	//If I have recieved SETUPCOORDS, forward them on, including my own
	if (sscanf_s(msg, "Message Ricky, SETUPCOORDS %d %d %d %d %d %d %d %d %d %d %d %d",
		&X[0], &Y[0], &X[1], &Y[1], &X[2], &Y[2], &X[3], &Y[3], &X[4], &Y[4], &X[5], &Y[5]) == 12){
		//Set my coordinates if not following friend
		if (following_friend != true){
			X[myOffset] = myX;
			Y[myOffset] = myY;
		}
		//Forward message on
		sprintf_s(newMessage, "SETUPCOORDS %d %d %d %d %d %d %d %d %d %d %d %d",
			X[0], Y[0], X[1], Y[1], X[2], Y[2], X[3], Y[3], X[4], Y[4], X[5], Y[5]);
		//send_message("Jack", newMessage);
		printf("Sending: %s\n", newMessage);
	}
	//If I have recieved COORDS, forward the message on THEN find my closest ship and head towards it
	if (sscanf_s(msg, "Message namehere, COORDS %d %d %d %d %d %d %d %d %d %d %d %d",
		&X[0], &Y[0], &X[1], &Y[1], &X[2], &Y[2], &X[3], &Y[3], &X[4], &Y[4], &X[5], &Y[5]) == 12){
		//Resend the setting up coordinate message
		sprintf_s(newMessage, "COORDS %d %d %d %d %d %d %d %d %d %d %d %d",
			X[0], Y[0], X[1], Y[1], X[2], Y[2], X[3], Y[3], X[4], Y[4], X[5], Y[5]);
	//	send_message("Broadberry", newMessage);
		printf("Sending: %s\n", newMessage);
		//Find Closest Friend
		int closest;
		closest = calculateClosest(X, Y);
		//Follow Closest Friend if not following friend
		if (following_friend != true){
			headTowardsCoordinate(X[closest], Y[closest], 2);
		}
	}
}

	
	


	//if (number_of_ships > 1)
	//{
	//	fire_at_ship(shipX[1], shipY[1]);
	//}



/*************************************************************/
/********* Your tactics code ends here ***********************/
/*************************************************************/



void communicate_with_server()
{
	char buffer[4096];
	int  len = sizeof(SOCKADDR);
	char chr;
	bool finished;
	int  i;
	int  j;
	int  rc;
	char* p;

	sprintf_s(buffer, "Register  %s,%s,%s", STUDENT_NUMBER, STUDENT_FIRSTNAME, STUDENT_FAMILYNAME);
	sendto(sock_send, buffer, strlen(buffer), 0, (SOCKADDR *)&sendto_addr, sizeof(SOCKADDR));

	while (true)
	{
		if (recvfrom(sock_recv, buffer, sizeof(buffer)-1, 0, (SOCKADDR *)&receive_addr, &len) != SOCKET_ERROR)
		{
		p = ::inet_ntoa(receive_addr.sin_addr);

			if ((strcmp(IP_ADDRESS_SERVER, "127.0.0.1") == 0) || (strcmp(IP_ADDRESS_SERVER, p) == 0))
			{
				i = 0;
				j = 0;
				finished = false;
				number_of_ships = 0;

				while ((!finished) && (i<4096))
				{
					chr = buffer[i];

					switch (chr)
					{
					case '|':
						InputBuffer[j] = '\0';
						j = 0;
						sscanf_s(InputBuffer,"%d,%d,%d,%d", &shipX[number_of_ships], &shipY[number_of_ships], &shipHealth[number_of_ships], &shipFlag[number_of_ships]);
						number_of_ships++;
						break;

					case '\0':
						InputBuffer[j] = '\0';
						sscanf_s(InputBuffer,"%d,%d,%d,%d", &shipX[number_of_ships], &shipY[number_of_ships], &shipHealth[number_of_ships], &shipFlag[number_of_ships]);
						number_of_ships++;
						finished = true;
						break;

					default:
						InputBuffer[j] = chr;
						j++;
						break;
					}
					i++;
				}

				myX = shipX[0];
				myY = shipY[0];
				myHealth = shipHealth[0];
				myFlag = shipFlag[0];


				tactics();

				if (fire)
				{
					sprintf_s(buffer, "Fire %s,%d,%d", STUDENT_NUMBER, fireX, fireY);
					sendto(sock_send, buffer, strlen(buffer), 0, (SOCKADDR *)&sendto_addr, sizeof(SOCKADDR));
					fire = false;
				}

				if (moveShip)
				{
					sprintf_s(buffer, "Move %s,%d,%d", STUDENT_NUMBER, moveX, moveY);
					rc = sendto(sock_send, buffer, strlen(buffer), 0, (SOCKADDR *)&sendto_addr, sizeof(SOCKADDR));
					moveShip = false;
				}

				if (setFlag)
				{
					sprintf_s(buffer, "Flag %s,%d", STUDENT_NUMBER, new_flag);
					sendto(sock_send, buffer, strlen(buffer), 0, (SOCKADDR *)&sendto_addr, sizeof(SOCKADDR));
					setFlag = false;
				}

			}
		}
		else
		{
			printf_s("recvfrom error = %d\n", WSAGetLastError());
		}
	}

	printf_s("Student %s\n", STUDENT_NUMBER);
}


void fire_at_ship(int X, int Y)
{
	fire = true;
	fireX = X;
	fireY = Y;
}



void move_in_direction(int X, int Y)
{
	if (X < -2) X = -2;
	if (X >  2) X =  2;
	if (Y < -2) Y = -2;
	if (Y >  2) Y =  2;

	moveShip = true;
	moveX = X;
	moveY = Y;
}


void set_new_flag(int newFlag)
{
	setFlag = true;
	new_flag = newFlag;
}



int _tmain(int argc, _TCHAR* argv[])
{
	char chr = '\0';

	printf("\n");
	printf("Battleship Bots\n");
	printf("UWE Computer and Network Systems Assignment 2 (2013-14)\n");
	printf("\n");

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return(0);

	//sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	//if (!sock)
	//{	
	//	printf("Socket creation failed!\n"); 
	//}

	sock_send = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	if (!sock_send)
	{	
		printf("Socket creation failed!\n"); 
	}

	sock_recv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	if (!sock_recv)
	{	
		printf("Socket creation failed!\n"); 
	}

	memset(&sendto_addr, 0, sizeof(SOCKADDR_IN));
	sendto_addr.sin_family = AF_INET;
	sendto_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	sendto_addr.sin_port = htons(PORT_SEND);

	memset(&receive_addr, 0, sizeof(SOCKADDR_IN));
	receive_addr.sin_family = AF_INET;
	receive_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	receive_addr.sin_addr.s_addr = INADDR_ANY;
	receive_addr.sin_port = htons(PORT_RECEIVE);

	int ret = bind(sock_recv, (SOCKADDR *)&receive_addr, sizeof(SOCKADDR));
	//	int ret = bind(sock_send, (SOCKADDR *)&receive_addr, sizeof(SOCKADDR));
	if (ret)
	{
		printf("Bind failed! %d\n", WSAGetLastError());  
	}

	communicate_with_server();

	closesocket(sock_send);
	closesocket(sock_recv);
	WSACleanup();

	while (chr != '\n')
	{
		chr = getchar();
	}

	return 0;
}

