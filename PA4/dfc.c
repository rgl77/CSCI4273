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
#include <errno.h>
// #include <openssl/md5.h>



#define buf 4096

// Server struct with 4 DFS
struct ser_in{
	char * name[4];
	char * ip[4];
	char * port[4];
	char * username;
	char * password;
	char * files[10];
}server_info;


// Function that reads config 
int config(char * filename);
// Function that requests a file and receives it
int get(char * filename, int sock[]);
// Function that puts a file 
int put(char * filename, int sock[]);
// ls in the server
int list(int sock[]);


// int user_valid(int sock[]);


// reading the config file for the dfc
int config(char * filename){
	char buffer[100];
	FILE * fp_1;
	char s1[20];
	char s2[20];
	char s3[50];

// Trying to open the config file
	if((fp_1 = fopen(filename, "r")) == NULL){
		perror("Error opening the config file in dfc->config function.");
	}

// indexing the servers
	int count = 0;
	// Read until the end of the file
	while(!feof(fp_1)){
		fgets(buffer, 100 , fp_1);

		// If there's 'Server' in the line
		if(strstr(buffer, "Server")){
			server_info.ip[count]   = calloc(sizeof(buffer),sizeof(char));
			server_info.name[count] = calloc(sizeof(buffer),sizeof(char));
			server_info.port[count] = calloc(sizeof(buffer),sizeof(char));

			// Read 3 parts of the file
			sscanf(buffer, "%s %s %s", s1,s2,s3);
			// Splitting the ip address and the port with :
			char * t = strtok(s3, ":");
			strcpy(server_info.ip[count],t);
			t = strtok(NULL,":");
			strcpy(server_info.port[count], t);
			printf("%d Connected to server with ip: %s  port:%s\n", count,server_info.ip[count], server_info.port[count] );
			count++;
		}

		// If username is present
		else if(strstr(buffer, "Username")){
			server_info.username = calloc(sizeof(buffer),sizeof(char));
			sscanf(buffer, "%s",s1);
			char * t = strtok(s1,":");
			t = strtok(NULL,":");
			strcpy(server_info.username, t);
			printf("My username from config file is: %s\n",server_info.username );
		}

		// If password is present
		else if(strstr(buffer, "Password")){
			server_info.password = calloc(sizeof(buffer),sizeof(char));
			sscanf(buffer,"%s",s1);
			char *t = strtok(s1,":");
			t = strtok(NULL, ":");
			strcpy(server_info.password,t);
			printf("My password from config file is: %s\n",server_info.password );
		}

		else{
			printf("Wrong value in the config file in dfc.c \n" );
			exit(1);
		}
	}
	fclose(fp_1);
	return 0;
}


// Requesting a file from the server
int get(char * filename, int sock[]){
	// Getting the list of files in the socket
	list(sock);
	int flag = 0;

	// Assume there's maximum of 10 files
	for(int i = 0; i < 10; i++){
		// If there's file in the global variable
		if(server_info.files[i] != NULL){
			// Check whether it is the requested file or not
			if(strstr(server_info.files[i],filename) != NULL){
				// Found the file!
				flag = 1;
				break;
			}
			else{
				continue;
			}
		}
	}

	// If flag == 0, then it does not fully exist in the dfs
	if(flag == 0){
		printf("File is incomplete\n");
		return 0;
	}


	char buffer[buf];
	char buffer__[4][buf];
	long buf_size;
	int file_count[4];

	for(int i; i < 4; i++){
		if(read(sock[i], &file_count[i], sizeof(file_count[i])) < 0){
			printf("Error receiving the file.\n");
		}
		printf("%s\n",  file_count[i]);
	}

	for(int i = 0; i<4; i++){
		for(int j=0; j<file_count[i];j++){
			char cc;
			if(read(sock[i], &cc, sizeof(char)) < 0){
				printf("Error receiving the file\n");
			}
			printf("%c\n", cc);

			if(read(sock[i], (char*)&buf_size,sizeof(buf_size)) < 0){
				printf("Error receiving the file\n");
			}
			int byte_size;

			if((byte_size = read(sock[i],buffer,buf_size)) <= 0){
				printf("Error receiving the file\n");
			}
			printf("%s\n", buffer );
			int f_n = cc - '0';
			if(f_n == 1)
				strcpy(buffer__[0],buffer);
			if(f_n == 2)
				strcpy(buffer__[1],buffer);
			if(f_n == 3)
				strcpy(buffer__[2],buffer);
			if(f_n == 4)
				strcpy(buffer__[3],buffer);
			bzero(buffer,sizeof(buffer));
			buf_size = 0;
		}
	}

	for (int i = 0; i < 4; i++){
		printf("%s\n", buffer__[i] );
	}
	printf("\n");
	FILE *f_1 = fopen(filename, "wb");
	if(f_1 == NULL){
		printf("File does not exist\n");
		exit(1);
	}
	for(int i = 0; i < 4; i++){
		fprintf(f_1, "%s", buffer__[i]);
	}
	return 0;
}

// // PUT FUNCTION '''
// int put(char * filename, int sock[]){
// 	int byte_size;
// 	FILE *fp;
// 	int blok_len;
// 	long file_len;
// 	char * part[4];
// 	int flag = 0;
// 	size_t n = 0;
// 	int c = 0;
// 	unsigned char out[MD5_DIGEST_LENGTH];
// 	MD5_CTX mdContext;
// 	unsigned char * hash_value;
// 	unsigned long hash_r = 0;
// 	int md5_table[4][4][2] =
// 	{
// 		{
// 			{0,1},{1,2},{2,3},{3,0}
// 		},
// 		{
// 			{3,0},{0,1},{1,2},{2,3}
// 		},
// 		{
// 			{2,3},{3,0},{0,1},{1,2}
// 		},
// 		{
// 			{1,2},{2,3},{3,0},{0,1}
// 		}
// 	};
// 	fp = fopen(filename, "r+b");
// 	if(fp == NULL)
// 	{
// 		printf("file does not exist!");
// 		exit(1);
// 	}
// 	fseek(fp, 0l, SEEK_END );
// 	file_len = ftell(fp);
// 	rewind(fp);
// 	blok_len = file_len/4;
// 	printf("%d %d\n", file_len, blok_len );
// 	if((file_len%2) != 0){
// 		flag = 1;
// 	}
// 	printf("flag %d\n",flag );
// 	for (int i=0; i < 4; i++)
// 		if (i == 3 && flag == 1){
// 					part[i] = calloc(sizeof(blok_len)+1, sizeof(char));
// 				}
// 		else
// 					part[i] = calloc(sizeof(blok_len), sizeof(char));

// 	int j = 0;
// 	MD5_Init(&mdContext);
// 	while((c = fgetc(fp)) != EOF ){
// 			part[j][n++] = (char) c;
// 			MD5_Update(&mdContext, &c, 1);
// 			if( j < 3){
// 				if(n >= blok_len){
// 					j++;
// 					n = 0;
// 			}
// 		}
// 	}

// 	MD5_Final(out,&mdContext);
// 	char temporary[4];
// 	hash_value = calloc(sizeof(MD5_DIGEST_LENGTH), sizeof(char));
// 	for (int i=0; i < MD5_DIGEST_LENGTH; i++){
// 		sprintf(temporary, "%02x",(unsigned int)out[i]);
// 		strcat(hash_value, temporary );
// 	}
// 	printf("%s\n", hash_value);
// 	char first_five[5];
// 	strncpy(first_five, hash_value, 5);
// 	hash_r =  (long)strtol(first_five, NULL, 16)%4;
// 	printf("%lu\n", hash_r );
// 	printf("%s\n",part[0] );
// 	printf("%s\n",part[1] );
// 	printf("%s\n",part[2] );
// 	printf("%s\n",part[3] );
// 	char ch;
// 	int e_b = 0;
// 	int datasize;
// 	int part_number;
// 	for(int j=0;j < 4; j++ ){
// 		for(int t=0; t < 2; t++){
// 			if (md5_table[hash_r][j][t] == 3 && flag == 1)
// 				e_b = 1;
// 			else e_b = 0;
// 			printf("%s  ",part[md5_table[hash_r][j][t]] );
// 			printf("%d\n",md5_table[hash_r][j][t]);
// 			datasize = strlen(part[md5_table[hash_r][j][t]]);
// 			printf("%d\n", datasize );
// 			datasize = htonl(datasize);
// 			part_number = (md5_table[hash_r][j][t]);
// 			if(write(sock[j], &part_number,sizeof(part_number)) < 0)
// 				perror("error writing1");
// 			if(write(sock[j], &datasize,sizeof(datasize)) < 0)
// 				perror("error writing2");
// 			if(write(sock[j], part[md5_table[hash_r][j][t]], strlen(part[md5_table[hash_r][j][t]])) < 1)
// 				perror("Eror sending the file.");
// 		}
// 	}

// 	fclose(fp);
// 	return 0;
// }

// LIST FUNCTION '''
int list(int sock[]){
	puts("In client and list");
	char * f_name[50] = {NULL};
	int nbytes,buflen,j=0;
	char buff[1024];
	puts("1..");
	puts(sock);
	sleep(1);
	// puts("woken up")
	for(int i=0; i<4; i++){
		puts("1.1....");
		if(nbytes = read(sock[i], (char*)&buflen, sizeof(buflen)) < 0)
			perror("error reading");
		buflen = ntohl(buflen);
		if(nbytes = read(sock[i], buff, buflen) < 0)
			perror("error reading");
		char *token = strtok(buff, "\n");
		puts("1.2....");
		while(token!=NULL){
			puts("1.2.1...");
			size_t token_size = strlen(token);
			// int  = 0;
			puts("1.2.2...");
			if(isdigit(token[strlen(token)-1])){
				f_name[j] = calloc(strlen(token), sizeof(char));
				strncpy(f_name[j],token, strlen(token));
						}
				token = strtok(NULL, "\n");
				j++;
		}
				puts("1.2.3...");
				bzero(buff,sizeof(buff));
				buflen = 0;
	}
	puts("2...");
	int counter = 0, flag = 0;
	char * fname_d[20] = {NULL};
	// int flag = 0;
	printf("%s\n","All Files:" );
	for(int i=0; i<50; i++){
			if(f_name[i] != NULL){
			printf("%s\n",f_name[i] );
		}
	}
	// printf("\n\n");
	for(int i=0; i<50; i++){
		if(f_name[i] != NULL){
			//printf("%s %d\n",f_name[i], i );
			for(int j=0; j<20; j++){
				if(fname_d[j] != NULL){
					if(strstr(f_name[i], fname_d[j]) != NULL){
						flag = 1;
						break;
					}
				else
					flag = 0;
				}
			}
			if(flag == 0){
					int t = sizeof(f_name[i]) - 3;
					fname_d[counter] = calloc(t, sizeof(char));
					strncpy(fname_d[counter], f_name[i], t);
					counter++;
			}
		}
	}
	int p_number[10][50] = {0};
	for(int i=0; i<20; i++){
		if(fname_d[i] != NULL){
		for(int j=0; j<50; j++){
			if(f_name[j] != NULL){
				if(strstr(f_name[j], fname_d[i]) != NULL){
						p_number[i][j] = f_name[j][strlen(f_name[j])-1] - '0';
				}
			}
		}
		}
	}
	printf("%s\n","Source Files:" );
	int f1,f2,f3,f4 = 0;
	int count = 0;
	for(int i=0;i<20;i++){
		if(fname_d[i] != NULL){
			for(int j=0;j<50; j++){
				if(p_number[i][j] != 0){
					if(p_number[i][j] == 1)
						f1 = 1;
					else if(p_number[i][j] == 2)
						f2 = 1;
					else if(p_number[i][j] == 3)
						f3 = 1;
					else if(p_number[i][j] == 4)
						f4 = 1;
					}
			}
		}
		else
			continue;

		if(f1 && f2 && f3 && f4){
			printf("%s\n",fname_d[i] );
			server_info.files[count] = calloc(strlen(fname_d[i]), sizeof(char));
			strcpy(server_info.files[count], fname_d[i]);
			count++;
		}
		else
			printf("%s [incomplete]\n", fname_d[i] );
	}
	return 0;
}

// main
int main(int argc, char *argv[]){
	char config_file[100];
	int number_of_bytes;                             // number of bytes send by sendto()
	int sock[4];                               //this will be our socket
	struct sockaddr_in dfs[4], tt;              //"Internet socket address structure"
	int remote_len = sizeof(tt);
	puts("main 1");
	if(argc != 2){
		printf("usage: <config_file>\n");
		EXIT_FAILURE;
	}
	else{
		strcpy(config_file, argv[1]);
	}
	if( config(config_file) < 0){
		perror("error in read_config\n");
	}
	puts("main 2");
	for (int i=0; i<4; i++){
		bzero(&dfs[i],sizeof(dfs[i]));               //zero the struct
		dfs[i].sin_family = AF_INET;                 //address family
		dfs[i].sin_port = htons(atoi(server_info.port[i]));      //sets port to network byte order
		dfs[i].sin_addr.s_addr = inet_addr(server_info.ip[i]); //sets remote IP address
		if ((sock[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			printf("unable to create socket");
		}
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		if (setsockopt(sock[i], SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
						sizeof(timeout)) < 0)
				perror("setsockopt failed\n");
		if (connect(sock[i], (struct sockaddr *) &dfs[i], sizeof(dfs[i]))<0) {
		 perror("Problem in connecting to the server");
		 exit(3);
		}
	}
	puts(sock);
	puts("main 3");
	char user_input[100], command[10], file_name[20], first_write[100];
	fgets(user_input, 30, stdin);
	sscanf(user_input, "%s %s", command, file_name);
	strtok(user_input, "\n");
	sprintf(first_write, "%s %s %s", user_input, server_info.username, server_info.password);
	printf("%s\n", first_write );
	int first_write_len = htonl(strlen(first_write));
	for(int i=0; i<4; i++){
		if (write(sock[i], (char*)&first_write_len, sizeof(first_write_len)) < 0)
		{
			perror("sendto");
			exit(1);
		}
		if (write(sock[i], first_write, strlen(first_write)) < 0)
		{
			perror("sendto");
			exit(1);
		}
	}
	puts("main 4");
	puts(sock);
	puts("------");
	if((strcmp(command, "get")) == 0){
		if(get(file_name, sock) < 0)
			perror("error calling get_func function");
	}
	// else if((strcmp(command, "put")) == 0){
	// 	if(put(file_name, sock) < 0)
	// 		perror("error calling put_func function");
	// }
	else if((strcmp(command, "list")) == 0){
		if(list(sock) < 0)
			perror("error calling get_func function");
	}
	else
		exit(1);

	return 0;
}

