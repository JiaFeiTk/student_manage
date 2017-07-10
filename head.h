#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define N 32

#define L  7 //login
#define A  1 //add user
#define D  2 //delete user
#define C  3 //change user
#define IA 4 //Inquire admin
#define IU 5 //Inquire user
#define Q  6 //quit 
#define LO 8 //look
#define CP 9 //look user

#define DATABASE "student.db"

typedef struct{
	int id;
	int  type;
	char name[N];
	char password[N];
}USER;

typedef struct{
	char name[N];
	char password[N];
	char addr[N];
	int  id;
	int  age;
	int  leve;
	int  aut;
	int type;
}INFO;

typedef struct node{
	USER user;
	struct node *next;
}linklist;

