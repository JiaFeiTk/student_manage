#include "head.h"

linklist * h_temp;//链表头节点
sqlite3 *db;

//函数声明
linklist *linklist_create();
int	delete_list(USER *user,linklist *h);
void do_login(int acceptfd, USER *user,INFO *info);
void *pthread_fun(void *arg);


int main(int argc, char *argv[])
{
	int sockfd, acceptfd;//套接字描述符
	struct sockaddr_in server_addr, client_addr;
	socklen_t len = sizeof(server_addr);
	char buf[N] = {0};

	pthread_t tid;//线程id
	char *errmsg; //sqlite 使用
	INFO auser;//默认管理员添加使用

	char sql[128] = {};//sql语句使用

	int flag = 0;//判断数据是否存在

	signal(SIGPIPE, SIG_IGN);//处理管道破裂
	//signal(SIGALRM, SIG_IGN);

	linklist *h = linklist_create();//创建头节点
	h_temp = h;//全局变量赋值

	//判断参数是否正确
	if(argc < 3){
		printf("parameter:%s <ip> <port>\n",argv[0]);
		exit(-1);
	}

	//flag标志
	if(fopen(DATABASE,"r") == NULL){
		flag = 1;	
	}

	//创建或者打开数据库
	if(sqlite3_open(DATABASE,&db) != SQLITE_OK){
		printf("error:%s\n",sqlite3_errmsg(db));
		exit(-1);
	}

	//创建表结构
	if(sqlite3_exec(db,"create table stu (id int primary key, \
		name char,password char,addr char,  \
				age int,leve int,aut int);",NULL,NULL, \
			&errmsg) != SQLITE_OK){
				printf("%s\n",errmsg);
			}

	//添加默认管理员
	if(flag == 1){

		strcpy(auser.name, "admin");
		strcpy(auser.password, "1");
		strcpy(auser.addr, "jia");
		auser.id = 1001;
		auser.age = 50;
		auser.leve = 1;
		auser.aut = 1;	

		sprintf(sql, "insert into stu values(%d,'%s', '%s','%s', %d, %d ,%d)",\
				auser.id,auser.name, auser.password,auser.addr,auser.age, \
				auser.leve,auser.aut);

		if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK){
			printf("%s\n", errmsg);
		}
	}	

	//创建套接字
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
		perror("fail to socket");
	}

	//清空结构体
	bzero(&server_addr,len);	

	//填充网络信息结构体
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));

	//绑定
	if(bind(sockfd,(struct sockaddr *)&server_addr,len) < 0){
		perror("fail to bind");
	}
	//监听
	if(listen(sockfd,5) < 0){
		perror("fail to listen");
	}


	while(1){

		acceptfd = accept(sockfd,(struct sockaddr *)&client_addr,&len);
		if(acceptfd < 0){
			perror("fail to accept");
		}		

		//创建线程
		if(pthread_create(&tid,NULL,pthread_fun,&acceptfd) != 0){
			perror("fail to pthread_create");
		}	

	}
	return 0;
}


//线程
void *pthread_fun(void *arg)
{
	USER user;
	INFO info;
	int acceptfd = *(int *)arg;
	do_login(acceptfd,&user,&info);
	delete_list(&user,h_temp);//删除断线用户

}

//网络连接完成后调用
void do_login(int acceptfd,USER *user,INFO *info)
{
	linklist *h = h_temp;
	linklist *p = h_temp->next;

	int flags = 0;//新用户标志位
	char sqlstr[128] = {0};//sql语句调用

	char *errmsg,**result;//sqlite3_get_table使用
	int nrow,ncolumn;
	int i = 0;
	int j = 0;
	int index = 0;
	int indexf = 0;

	int count = 0;//修改用户信息条数

loop:	//接收用户登录信息
	if(recv(acceptfd, user, sizeof(USER), 0) <0 ){
		close(acceptfd);
		printf("SSFDSFDSF");
	}
	//用户登录查询
	sprintf(sqlstr,"select * from stu where id = %d and password = '%s'",user->id,user->password);

	if(sqlite3_get_table(db, sqlstr, &result, &nrow, &ncolumn, &errmsg) != SQLITE_OK){
		printf("error:%s\n",errmsg);
	}else{
		if(nrow <= 0){
			strcpy(info->name, "fail");
			send(acceptfd, info, sizeof(INFO), 0);
			sqlite3_free_table(result);
			goto loop;   //查询失败，重新接收用户登录信息
		}

		sqlite3_free_table(result);//释放result空间

		//查询成功
		if(nrow > 0){
			//还原初始值
			h = h_temp;
			p = h_temp->next;
			flags = 0;
			//遍历已登录陆用户
			while(p != NULL){
				if(p->user.id == user->id){
					flags = 1;
					info->aut = 0;
					strcpy(info->name, "已连接");
					send(acceptfd,info,sizeof(INFO),0);
					goto loop;//查询到已经有用户登录，重新接收用户登录信息
					
				}else{
					flags = 0;
					p = p->next;
				}
			}
			
			//如果是新用户登录
			if(flags == 0){
				linklist *p = h_temp->next;	
				//用户插入已登陆用户链表
				linklist *temp = (linklist *)malloc(sizeof(linklist));
				temp->user = *user;	
				temp->next = h->next;
				h->next = temp;
				info->aut = atoi(result[ncolumn + 6]);
				send(acceptfd,info,sizeof(INFO),0);
			}


			while(recv(acceptfd,info,sizeof(INFO),0) > 0){ 

				switch(info->type){
					case A: //增加用户
						info->aut = 2;
						strcpy(info->password ,"1");
						sprintf(sqlstr, "insert into stu values(%d,'%s', \
										'%s','%s', %d, %d ,%d)",info->id,info->name,\
										info->password,info->addr,info->age,info->leve,info->aut);

						if(sqlite3_exec(db, sqlstr, NULL, NULL, &errmsg) != SQLITE_OK){
							strcpy(user->name, "fail");
						}else{
							strcpy(user->name, "OK");
						}
						send(acceptfd, user, sizeof(USER), 0);
						break;
						
					case D://删除用户
						sprintf(sqlstr, "delete from stu where id = %d", info->id);

						if(sqlite3_exec(db, sqlstr, NULL, NULL, &errmsg) != SQLITE_OK){
							strcpy(user->name, "fail");
						}
						else{
							strcpy(user->name, "OK");
						}
						send(acceptfd, user, sizeof(USER), 0);
						break;
						
					case C://信息更改
						sprintf(sqlstr, "update stu set addr = '%s' where id = %d", info->addr, info->id);
						if(sqlite3_exec(db, sqlstr,  NULL, NULL, &errmsg) != SQLITE_OK){
							count = count;
						}else{
							count++;
						}

						sprintf(sqlstr, "update stu set age = %d where id = %d", info->age, info->id);
						if(sqlite3_exec(db, sqlstr,  NULL, NULL, &errmsg) != SQLITE_OK){
							count = count;
						}else{
							count++;
						}

						sprintf(sqlstr, "update stu set leve = %d where id = %d", info->leve, info->id);
						if(sqlite3_exec(db, sqlstr,  NULL, NULL, &errmsg) != SQLITE_OK){
							count = count;
						}else{
							count++;
							user->type = count;
						}
						send(acceptfd, user, sizeof(USER), 0);
						break;

					case IA://管理员查询
						sprintf(sqlstr,"select * from stu");
						if(sqlite3_get_table(db, sqlstr, &result, &nrow, &ncolumn, &errmsg) != SQLITE_OK){
							printf("%s\n", errmsg);
						}else{
						//	printf("select dpne.\n");
						}

						for(j = 0; j < ncolumn; j++){
							strcpy(user->name, result[j]);
							send(acceptfd, user, sizeof(USER), 0);
						}
						strcpy(user->name, "\n");
						send(acceptfd, user, sizeof(USER), 0);

						index = ncolumn;
						indexf = ncolumn;
						for(i = 0; i < nrow; i++){
							for(j = 0; j < ncolumn; j++){
								strcpy(user->name, result[indexf++]);
								send(acceptfd, user, sizeof(USER), 0);
							}
							strcpy(user->name, "\n");
							send(acceptfd, user, sizeof(USER), 0);
						}

						sqlite3_free_table(result);
						break;
						
					case IU://用户查询
							
						sprintf(sqlstr,"select * from stu where id = %d",user->id);
						if(sqlite3_get_table(db, sqlstr, &result, &nrow, &ncolumn, &errmsg) != SQLITE_OK){
							printf("error:%s\n",errmsg);
						}
						for(j = 0; j < ncolumn; j++){
							strcpy(user->name, result[j]);
							send(acceptfd, user, sizeof(USER), 0);
						}
						strcpy(user->name, "\n");
						send(acceptfd, user, sizeof(USER), 0);

						index = ncolumn;
						indexf = ncolumn;
						for(i = 0; i < nrow; i++){
							for(j = 0; j < ncolumn; j++){
								strcpy(user->name, result[indexf++]);
								send(acceptfd, user, sizeof(USER), 0);
							}
							strcpy(user->name, "\n");
							send(acceptfd, user, sizeof(USER), 0);
						}
						
						sqlite3_free_table(result);
						break;
						
					case CP://用户密码更改
						sprintf(sqlstr, "update stu set password = '%s' \
											where id = %d", info->password, info->id);
						if(sqlite3_exec(db, sqlstr,  NULL, NULL, &errmsg) != SQLITE_OK){
							strcpy(user->name, "fail");
						}else{
							strcpy(user->name, "OK");
						}
						send(acceptfd, user, sizeof(USER), 0);
						break;
				}
			}
		}
	}
}

//创建空列表
linklist *linklist_create()
{
	linklist *h = (linklist *)malloc(sizeof(linklist));

	h->next = NULL;

	return h;
}

//删除已登陆用户
int	delete_list(USER *user,linklist *h)
{
	linklist *p = h;


	while(p->next != NULL){
		if(user->id == p->next->user.id){
			linklist *temp = p->next;
			p->next = temp->next;
			free(temp);
			temp = NULL;
			break;
		}
		p = p->next;
	}
}
