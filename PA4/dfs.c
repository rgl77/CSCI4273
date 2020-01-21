#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <dirent.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#define MAXLINE 4096

struct user_array{
	char * username[10];
	char * password[10];
}user_array;

struct server_details{
	char DFS_SERVER_NAME[20];
	char portnum[20];
	int DFS_num;
}server_details;

int prepare_with_config(char * conf_fn)
{
	 char buffer[100];
	 FILE * fp;
	 char string1[20];
	 char string2[20];
	 char string3[50];
	 if((fp = fopen(conf_fn, "r")) == NULL)
	 		perror("config file");

	int count = 0;
	while(!feof(fp))
	{
		fgets(buffer, 100, fp);
    user_array.username[count] = calloc(sizeof(buffer), sizeof(char));
    user_array.password[count] = calloc(sizeof(buffer), sizeof(char));
    sscanf(buffer, "%s %s", user_array.username[count], user_array.password[count]);
    printf("%s %s\n",user_array.username[count], user_array.password[count] );
    count++;
  }
	return 0;
}

int help_user(int connection){
  	char buf[MAXLINE];
	int buflen = 0;
	int part_number;
	int nbytes;
	char command[4];
	char file_name[30];
	char client_username[30];
	char client_password[30];
  	if(nbytes = read(connection, (char*)&buflen, sizeof(buflen)) < 0)
		perror("dfs 69");
	buflen = ntohl(buflen);
	// printf("dfs 71: %d\n", buflen);
    if((nbytes = read(connection, buf, buflen)) < 0)
		perror("error reading");
	// printf("The client says %s\n", buf);
	char temp[4];
	strncpy(temp,buf, 4);
	char extra[10];
	if(strcmp(temp,"list")==0)
			sscanf(buf, "%s %s %s",command, client_username, client_password );
	else
		sscanf(buf, "%s %s %s %s",command, file_name,client_username,
		 client_password );
	// puts("-----");
	// puts(user_array.username);
	// puts(user_array.password);
	// puts(client_username);
	// puts(client_password);
	// puts("----");
	int flag = 0;
	for(int i=0; i<10; i++){
		if(user_array.username[i] == NULL)
			break;
		// if(strcmp(user_array.username[i],client_username)==0 &&
		//  strcmp(user_array.password[i], client_password)==0 )
			// puts("-----");
			// puts(user_array.username[i]);
			// puts(user_array.password[i]);
			// puts(client_username);
			// puts(client_password);
			// puts("----");
			if(strstr(client_username,user_array.username[i]) != NULL &&
				strstr(client_password,user_array.password[i]) != NULL){
			 flag = 1;
			 break;
		 }
		 else
		 	continue;
	}
	if(flag == 0){
		printf("%s\n","dfs 110" );
		return -1;
	}
	struct stat st = {0};
	char dir[30];
	sprintf(dir, "./%s", client_username );
	printf("%s\n", dir);
	if (stat(dir, &st) == -1){
		mkdir(dir, 0777);
	}

	// // PUT //
	// if(strcmp(command, "put") == 0){
	// 	bzero(buf,sizeof(buf));
	// 	for(int i=0; i<2; i++){
	// 		buflen = 0;
	// 		if(read(connection, &part_number, sizeof(part_number)) < 0)
	// 			printf("dfs 127\n");
	// 		printf("%d\n",part_number );

	// 		if(read(connection, &buflen, sizeof(buflen)) < 0)
	// 			printf("dfs 131\n");
	// 		buflen = ntohl(buflen);
	// 		printf("%d\n",buflen );
	// 		if(read(connection, buf, buflen) < 0)
	// 			printf("dfs 135\n");
	// 		printf("%s\n",buf );
	// 		char dir1[50];
	// 		sprintf(dir1, "%s/%s.%d",dir, file_name, part_number+1);
	// 		printf("%s\n",dir1 );
	// 		FILE *f = fopen(dir1, "wb");
	// 		if(f == NULL)
	// 			perror("dfs 142\n");
	// 		// 	return -1;
	// 		// }
	// 		fprintf(f, "%s", buf);
	// 		fclose(f);
	// 		bzero(buf,sizeof(buf));
	// 		buflen = 0;
	// 		part_number = 0;
	// 	}
	// }

// GET //
	if(strcmp(command, "get") == 0){
		printf("%s\n","in LIST:" );
		char str[200];
		DIR *d;
		struct dirent *dir2;

		if((d = opendir(dir)) == NULL)
			perror("cannot opendir");
		if (d)
		{
			while((dir2 = readdir(d)) != NULL)
			{
				strcat(str, dir2->d_name);
				strcat(str, "\n");
			}
			printf("%s", str );
			closedir(d);
		}
		int list_len = htonl(strlen(str));
		if (write(connection, (char*)&list_len, sizeof(list_len)) < 0)
		{
			exit(1);
		}
		if (write(connection, str, strlen(str)) < 0)
		{
			exit(1);
		}
		char * f_name[50] = {NULL};
		char *token = strtok(str, "\n");
		int j = 0;
		while(token!=NULL){
			size_t token_size = strlen(token);
			int counter = 0;
			if(isdigit(token[strlen(token)-1])){
				f_name[j] = calloc(strlen(token), sizeof(char));
				strncpy(f_name[j],token, strlen(token));
						}
				token = strtok(NULL, "\n");
				j++;
		}
		int file_count = 0;
		int flag1,flag2,flag3,flag4 = 0;
		for(int i=0;i<50; i++){
			if(f_name[i] != NULL && strstr(f_name[i],file_name) != NULL){
				char c1 = (f_name[i][strlen(f_name[i])-1]);
				int c2 = c1 - '0';
				if(c2==1 && flag1 == 0 ){
					flag1 = 1;
					file_count++;
				}
				if(c2==2 && flag2 == 0){
					flag2 = 1;
					file_count++;
				}
				if(c2==3 && flag3 == 0){
					flag3 = 1;
					file_count++;
				}
				if(c2==4 && flag4 == 0){
					flag4 = 1;
					file_count++;
				}
			}
		}
		int flag[4];
		if(write(connection, &file_count, sizeof(file_count)) < 0)
			perror("cannot write to connection");

		for(int i=0; i<50; i++){
			if(f_name[i] != NULL && strstr(f_name[i],file_name) != NULL){
			char cc = (f_name[i][strlen(f_name[i])-1]);
			int cc2 = cc - '0';
			// printf("%s %s\n", f_name[i], file_name );
			char file_dir[50];
			// sprintf(file_dir, "%s/%s", dir, f_name[i] );
			// printf("%s\n", file_dir );
			FILE *fp = fopen(file_dir, "r+b");
			if(fp == NULL)
				perror("file error");
			fseek(fp, 0l, SEEK_END );
			long file_size = ftell(fp);
			rewind(fp);
			int c;
			int n = 0;
			bzero(buf,sizeof(buf));
			if(fp != NULL){
				while((c = fgetc(fp)) != EOF ){
					buf[n++] = (char) c;
				}
			}
			// printf("%c\n", f_name[i][strlen(f_name[i])-1] );
			if(flag[cc2-1] != 1){
			if(write(connection, &cc, sizeof(char)) < 0)
				perror("dfs 247");
			if(write(connection, (char *)&file_size, sizeof(file_size)) < 0)
				perror("dfs 249");
			printf("%s\n", buf );
			if(write(connection, buf, file_size) < 0)
				perror("dfs 252");
			flag[cc2-1] = 1;
			}
			fclose(fp);
			}
		}
	}

// lIST //
	if(strcmp(command, "list") == 0){
		printf("%s\n","in LIST:" );
		char str[200];
		DIR *d;
		struct dirent *dir2;
		puts(dir);
		if((d = opendir(dir)) == NULL)
			perror("cannot opendir");
		if (d)
		{
			while((dir2 = readdir(d)) != NULL)
			{
				strcat(str, dir2->d_name);
				strcat(str, "\n");
			}
			printf("%s", str );
			closedir(d);
		}
		int list_len = htonl(strlen(str));
		if (write(connection, (char*)&list_len, sizeof(list_len)) < 0)
		{
			exit(1);
		}
		if (write(connection, str, strlen(str)) < 0)
		{
			exit(1);
		}
	}
	return 0;
}

int main (int argc, char * argv[] ){
  char conf_fn[20] = "dfs.conf";
  pid_t pid;
	int sock, connection, port;
	struct sockaddr_in sin, remote;     
	int remote_len = sizeof(remote);
	unsigned int remote_length;         
	int nbytes;                        
	char command[10];

  if(argc != 3){
    printf("<DFS_SERVER_NAME> <portnum>\n");
    exit(1);
  }
	strcpy(server_details.DFS_SERVER_NAME, argv[1]);
  strcpy(server_details.portnum, argv[2]);
	server_details.DFS_num = server_details.DFS_SERVER_NAME[3] - '0';
  if( prepare_with_config(conf_fn) < 0){
		perror("cannot read config\n");
	}

  bzero(&sin,sizeof(sin));                    
	sin.sin_family = AF_INET;                   
	sin.sin_port = htons(atoi(server_details.portnum));        
	sin.sin_addr.s_addr = INADDR_ANY;           

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("cannot create the socket for the server");
  }

  if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
  {
    perror("cannot bind the socket in the server.\n");
    exit(1);
  }

  if( listen(sock, 1) < 0)
	{
		printf("cannot listen to the socket");
	}
  while(1)
  {
    if ((connection = accept(sock, NULL, NULL)) < 0)
      perror("cannot accept connection");
    if ((pid = fork()) == 0)
    {
      if(close(sock) < 0)
        perror("Child process cannot close the socket");

      if(help_user(connection) < 0)
				printf("%s\n","Cannot process the user's request method.");
      if(close(connection) < 0)
        perror("Cannot close the connection in the child process.");

      exit(EXIT_SUCCESS);
    }
    if(close(connection) < 0)
      perror("Cannot close the connection in the parent process.");
    waitpid(-1, NULL, WNOHANG);
  }


  return 0;
}
