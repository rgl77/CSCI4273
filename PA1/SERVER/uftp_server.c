#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#define BUFSIZE 1024

int main(int argc, char **argv){
	int sockfd,portno,clientlen;
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	struct hostent *hostp;
	char *filename;
	char buf[BUFSIZE];
	char *hostaddrp;
	int optval;
	int n;
	char user;
	char prompt[BUFSIZE];
	char prompt_save[BUFSIZE];
	int flag = 0;
	if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
    }	
    portno = atoi(argv[1]);
    sockfd = socket(AF_INET,SOCK_DGRAM,0);
    if(sockfd <0){
    	printf("Error opening socket\n");
    	exit(1);
    }

    int g_check_pack = 0;
    int send_flag = 0;
    FILE *qwerty;
    int total = 0;
    int pack_size = 0;
    int total_check = 0;
    char A[10] = "YES";
    char *first_base;

    optval = 1;
    setsockopt(sockfd,SOL_SOCKET, SO_REUSEADDR, (const void*)&optval,sizeof(int));

    bzero((char *) &serveraddr, sizeof(serveraddr));
    //bzero(&serveraddr,sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)portno);

    if (bind(sockfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0){
    	printf("Error on binding\n");
    	exit(0);
    } 
    //error("ERROR on binding");
	clientlen = sizeof(clientaddr);
	while(1){
		bzero(buf,sizeof(buf));
		n = recvfrom(sockfd,prompt,sizeof(prompt),0,&clientaddr, &clientlen);
		strcpy(prompt_save,prompt);
		if(strcmp(prompt, "ls\n") == 0) {
            user = 'l';
        }
        else if(strcmp(prompt, "exit\n") == 0) {
            user = 'e';
        }
        else {
            first_base = strtok(prompt, " ");
            if(strcmp(first_base, "get") == 0) {
                user = 'g';
            }
            else if(strcmp(first_base, "put") == 0) {
                user = 'p';
            }
            else if(strcmp(first_base, "delete") == 0) {
                user = 'd';
            }
        }
		//user = prompt_check(prompt);
		switch(user){
			case 'd':
				filename = strtok(NULL,"\n");
				qwerty = fopen(filename,"wb");
				remove(filename);
				fclose(qwerty);
			break;

			case 'e':
				printf("Exiting\n" );
				exit(0);
			break;

			case 'g':
				filename = strtok(NULL,"\n");
				qwerty = fopen(filename,"rb");
				total = 0;
				while(!feof(qwerty)){
					pack_size = fread(buf,1,BUFSIZE-1,qwerty);
					n = sendto(sockfd,&pack_size,sizeof(int),0,(struct sockaddr *) &clientaddr, clientlen);
					n = sendto(sockfd,buf,sizeof(buf),0,(struct sockaddr *) &clientaddr, clientlen);
					total += pack_size;
				}
				fclose(qwerty);
				pack_size = -999;
				n = sendto(sockfd,&pack_size,sizeof(int),0,(struct sockaddr *) &clientaddr, clientlen);
				n = sendto(sockfd,&total,sizeof(int),0,(struct sockaddr *) &clientaddr, clientlen);
			break;

			case 'p':
				filename = strtok(NULL,"\n");
				qwerty = fopen(filename,"wb");
				total = 0;
				flag = 0;

				while(1){
					n = recvfrom(sockfd,&pack_size,sizeof(int),0,(struct sockaddr *) &clientaddr, clientlen);
					if(pack_size == -999){
						break;
					}
					bzero(buf,sizeof(buf));
					n = recvfrom(sockfd,&buf,sizeof(buf),0,(struct sockaddr *) &clientaddr, clientlen);
					printf("%s\n",buf );
					if(n > 0 ){
						total += pack_size;
					}
					fwrite(buf,1,pack_size,qwerty);
					if(pack_size < BUFSIZE-1){
						n = recvfrom(sockfd,&pack_size,sizeof(int),0,(struct sockaddr *) &clientaddr, clientlen);
						flag = 1;
						break;
					}
				}
				fclose(qwerty);
				n = recvfrom(sockfd,&total_check,sizeof(int),0,(struct sockaddr *) &clientaddr, clientlen);
				if(total != total_check  || total == 0 || flag == 0){
					strcpy(prompt,prompt_save);
					n = sendto(sockfd,prompt,sizeof(buf),0,(struct sockaddr *) &clientaddr, clientlen);	
				}
				else{
					n = sendto(sockfd,"YES",sizeof(buf),0,(struct sockaddr *) &clientaddr, clientlen);
				}
			break;

			case 'l':
				qwerty = fopen("dir.txt","wb");

				struct dirent **list;
				int scan;
				scan = scandir(".",&list,NULL,alphasort);
				if(scan<0){
					printf("cannot scan, there's an error\n");
					exit(1);
				}
				else{
					while(scan--){
						fprintf(qwerty, "%s\n", list[scan]->d_name );
						free(list[scan]);
					}
					fclose(qwerty);
					free(list);
				}
				qwerty = fopen("dir.txt","rb");;
				total = 0;
				while(!feof(qwerty)){
					pack_size = fread(buf,1,BUFSIZE-1,qwerty);
					n = sendto(sockfd,&pack_size,sizeof(int),0,(struct sockaddr *) &clientaddr, clientlen);
					n = sendto(sockfd,buf,sizeof(buf),0,(struct sockaddr *) &clientaddr, clientlen);
					total += pack_size;
				}
				fclose(qwerty);
				pack_size = -999;
				n = sendto(sockfd,&pack_size,sizeof(int),0,(struct sockaddr *) &clientaddr, clientlen);
			break;

			default:
				n = sendto(sockfd,prompt_save,sizeof(buf),0,(struct sockaddr *) &clientaddr, clientlen);
			break;
		}
	}

}