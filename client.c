#include "head.h"

int do_login(int sockfd, USER *user, INFO *info);
int do_admin(int sockfd,USER *user,INFO *info);
int do_user(int sockfd,USER *user,INFO *info);
int do_add(int sockfd,USER *user,INFO *info);
int do_delete(int sockfd,USER *user,INFO *info);
int do_change(int sockfd,USER *user,INFO *info);
int do_look(int sockfd,USER *user,INFO *info);
int do_chpwd(int sockfd,USER *user,INFO *info);

int main(int argc,char *argv[])
{
	int sockfd;

	USER user;
	INFO info;
	int choose;
	struct sockaddr_in server_addr;
	socklen_t len = sizeof(server_addr);

	if(argc < 3){
		printf("parameter:%s <ip> <port>\n",argv[0]);
		exit(-1);
	}

	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
		perror("fail to socket");
		exit(-1);
	}

	bzero(&server_addr,len);

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));

	if(connect(sockfd,(struct sockaddr *)&server_addr,len) < 0){
		perror("fail to connect");
		exit(-1);
	}

	//主程序
	while(1){
		printf("************************************\n");
		printf("******** 1: login   2: quit ********\n");
		printf("************************************\n");
		printf("please choose : ");
		scanf("%d",&choose);
		getchar();
		switch(choose){
			case 1:
				do_login(sockfd,&user,&info);
				break;
			case 2:
				close(sockfd);
				exit(0);
			default:
				break;
		}

	}
	return 0;
}

int do_login(int sockfd, USER *user, INFO *info)
{
	user->type = L;
	//输入name
	printf("-->input you id:");
	scanf("%d", &user->id);
	//输入password
	printf("-->input you pass world:");
	scanf("%s", user->password);
	//发送数据
	send(sockfd,user,sizeof(USER),0);
	//接受数据
	recv(sockfd,info,sizeof(INFO),0);
	printf("%s\n",info->name);
	if(info->aut == 1 || info->aut == 2){
		printf("login:OK\n");
	}

	if(info->aut == 1){
		do_admin(sockfd,user,info);
	}
	if(info->aut == 2){
		do_user(sockfd,user,info);
	}
	
}

int do_admin(int sockfd,USER *user,INFO *info)
{
	int n;
	while(1){
		printf("**************************************\n");
		printf("**1:add 2:del 3:change 4:look 5:quit**\n");
		printf("**************************************\n");
		printf("please choose : ");
		if(scanf("%d",&n) <=0){
			perror("fail to scanf");
			exit(-1);
		}

		switch(n){
			case 1:
			 	do_add(sockfd,user,info);
				break;
			case 2:
				do_delete(sockfd,user,info);
				break;
			case 3:
				do_change(sockfd,user,info);
				break;
			case 4:
				do_look(sockfd,user,info);
				break;
			case 5:
				close(sockfd);
				exit(0);
		}
	}
}

int do_user(int sockfd,USER *user,INFO *info)
{
	int n;
	while(1){
		printf("************************************\n");
		printf("******1:change 2:look 3:quit********\n");
		printf("************************************\n");
		printf("please choose : ");
		if(scanf("%d",&n) <=0){
			perror("fail to scanf");
			exit(-1);
		}

		switch(n){
			case 1:
				do_chpwd(sockfd,user,info);
				break;
			case 2:
				do_look(sockfd,user,info);
				break;
			case 3:
				close(sockfd);
				exit(0);
		}
	}
	
}

int do_add(int sockfd,USER *user,INFO *info)
{
	info->type = A;
	printf("input your id:");
	scanf("%d", &info->id);
	
	printf("input your name:");
	scanf("%s", info->name);
	
	printf("input your addr:");
	scanf("%s", info->addr);

	printf("input your age:");
	scanf("%d", &info->age);

	printf("input your leve:");
	scanf("%d", &info->leve);

	send(sockfd, info, sizeof(INFO), 0);
	
	recv(sockfd, user, sizeof(USER), 0);
	
	printf("register : %s\n", user->name);
}

int do_delete(int sockfd,USER *user,INFO *info)
{
	info->type = D;
	
	printf("input your want delete id:");
	scanf("%d", &info->id);
	send(sockfd, info, sizeof(INFO), 0);
	recv(sockfd, user, sizeof(USER), 0);
	printf("delete info : %s\n", user->name);
}

int do_change(int sockfd,USER *user,INFO *info)
{
	info->type = C;
	printf("input your want change id:");
	scanf("%d", &info->id);
	 
	printf("input your want change addr:");
	scanf("%s", info->addr);

	printf("input your want change age:");
	scanf("%d", &info->age);

	printf("input your want change leve:");
	scanf("%d", &info->leve);
	send(sockfd, info, sizeof(INFO), 0);
	recv(sockfd, user, sizeof(USER), 0);
	if(user->type == 3)
	printf("delete info : %s\n", user->name);
}

void handler(int sig)
{

}

int do_look(int sockfd,USER *user,INFO *info)
{
	int bytes;
	if(info->aut == 1){
		info->type = IA;
	}
	
	if(info->aut == 2){
		info->type =IU;
	}
	//设置网络超时
	//第一步：获取旧的行为
	struct sigaction act;
	if(sigaction(SIGALRM, NULL, &act) < 0)
	{
		printf("fail to sigaction");
	}

	//第二步：修改行为--自重启属性
	act.sa_handler = handler;
	act.sa_flags = act.sa_flags & (~SA_RESTART);

	//第三步：将新的行为写进去
	if(sigaction(SIGALRM, &act, NULL) < 0)
	{
		printf("fail to sigaction");
	}
	
	send(sockfd, info, sizeof(INFO), 0);
	alarm(1);
	while((bytes = recv(sockfd, user, sizeof(USER), 0)) > 0)
	{		
		if(!strcmp(user->name,"\n")){
			printf("%s",user->name);
		}else{
			printf("%-8s",user->name);
		}
	}
	
}

int do_chpwd(int sockfd,USER *user,INFO *info)
{
	info->type = CP;
	printf("input your id:");
	scanf("%d", &info->id);
	printf("input your new password:");
	scanf("%s", info->password);
	
	send(sockfd, info, sizeof(INFO), 0);
	recv(sockfd, user, sizeof(USER), 0);
	printf("change info : %s\n", user->name);
}
