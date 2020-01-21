#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <errno.h>
#include <dirent.h>


#define BUFSIZE 1024

void user_prompts(){
	printf("Here are the possible functionalities\n");
	printf("get [filename.type]\n");
	printf("put [filename.type]\n");
	printf("delete [filename.type]\n");
	printf("ls\n");
	printf("exit\n");
	printf("\n");
}

int main(int argc, char* argv[]){
	int sockfd, portno, n;
	int serverlen;
	char buf[BUFSIZE];
	struct sockaddr_in serveraddr;
	unsigned int from_length = sizeof(struct sockaddr_in);
	bzero(buf,sizeof(buf));
	char prompt[BUFSIZE],prompt_save[BUFSIZE],temp_buf[BUFSIZE];
	struct hostent *server;
	char *hostname;
	//char *user;
	char *filename;
	FILE *qwerty;
	int pack_size = 0;
	int total = 0;
	int send_flag = 0;
	int g_check_pack = 0;
	char *first_base;
	char user;
	if(argc != 3){
		fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
      	exit(0);
	}
	hostname = argv[1];
	portno = atoi(argv[2]);

	// Initialize the socket
	// AF_INET           -> address family
	// SOCK_DGRAM        -> UDP protocol
	// Internet protocol -> 0
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0){
		printf("ERROR opening soket.\n");
		exit(0);
		//error("ERROR opening socket");
	}
	/* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if(server == NULL){
    	fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }
    /* build the server's Internet address */

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
    	(char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);
  

    while(1){
    	user_prompts();
    	fgets(prompt,BUFSIZE,stdin);
    	strcpy(prompt_save,prompt);
if_fail_go_back_here: n = sendto(sockfd,prompt,sizeof(prompt),0,&serveraddr,sizeof(serveraddr));
    	if (n < 0) {
    		printf("ERROR in sendto\n");
    	}

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
    	switch(user){
    		case 'e':
    			printf("Exiting\n");
    			exit(0);
    		break;

    		case 'd':
    			printf("Deleted the requested file from the server.\n");
    		break;

    		case 'g':
    			total = 0;
    			send_flag = 0;
    			filename = strtok(NULL,"\n");
    			qwerty = fopen(filename,"w+");
    			while(1){
    				n = recvfrom(sockfd,&pack_size,sizeof(int),0,&serveraddr,&serverlen);
    				if(pack_size == -999){
    					break;
    				}
    				bzero(buf,sizeof(buf));
    				n = recvfrom(sockfd,buf,sizeof(buf),0,&serveraddr,&serverlen);
    				if(n >= 0){
    					total += pack_size;
    				}
    				fwrite(buf,1,pack_size,qwerty);
    				if(pack_size<BUFSIZE-1){
    					n = recvfrom(sockfd,&pack_size,sizeof(int),0,&serveraddr,&serverlen);
    					send_flag = 1;
    					break;
    				}
    			}
    			fclose(qwerty);
    			n = recvfrom(sockfd,&g_check_pack,sizeof(int),0,&serveraddr,&serverlen);
    			if(total != g_check_pack || send_flag == 0 || total == 0){
    				strcpy(prompt,prompt_save);
    				goto if_fail_go_back_here;
    			}
    			printf("We have received the file from the server\n");
    		break;

    		case 'p':
	    		send_flag = 0;
	    		total = 0;
    			filename = strtok(NULL,"\n");
    			qwerty = fopen(filename,"rb");
    			//bzero(buf,sizeof(buf);
    			bzero(buf,sizeof(buf));
    			while(!feof(qwerty)){
    				printf("In while loop\n");
    				pack_size = fread(buf,1,BUFSIZE-1,qwerty);
    				n = sendto(sockfd,&pack_size,sizeof(int),0,&serveraddr,&serverlen);
    				n = sendto(sockfd,&buf,sizeof(buf),0,&serveraddr,&serverlen);
    				printf("%s\n",&buf );
    				total += pack_size;
    			}
    			fclose(qwerty);
    			pack_size = -999;
    			n = sendto(sockfd,&pack_size,sizeof(int),0,&serveraddr,&serverlen);
    			n = sendto(sockfd,&total,sizeof(int),0,&serveraddr,&serverlen);
    			n = recvfrom(sockfd,prompt,sizeof(buf),0,&serveraddr,&serverlen);
    			if(strcmp(prompt,"YES") != 0){
    				goto if_fail_go_back_here;
    			}
    			printf("File successfully sent to the server\n");
    		break;

    		case 'l':
    			qwerty = fopen("dir.txt","wb");
    			send_flag = 0;
    			while(1){
    				n = recvfrom(sockfd,&pack_size,sizeof(int),0,&serveraddr,&serverlen);
    				if(pack_size == -999){
    					break;
    				}
    				bzero(buf,sizeof(buf));
    				n = recvfrom(sockfd,buf,sizeof(buf),0,&serveraddr,&serverlen);
    				fwrite(buf,1,pack_size,qwerty);
    				total += pack_size;
    				if(pack_size < BUFSIZE -1){
    					send_flag = 1;
    					n = recvfrom(sockfd,&pack_size,sizeof(int),0,&serveraddr,&serverlen);
    					break;
    				}
    			}
    			fclose(qwerty);
    			if(send_flag == 0 || total == 0){
    				strcpy(prompt,prompt_save);
    				sleep(5);
    				goto if_fail_go_back_here;
    			}
    			qwerty = fopen("dir.txt","rb");
    			printf("ls from the server:\n");
    			while(!feof(qwerty)){
    				fscanf(qwerty,"%s\n",temp_buf);
    				printf("%s\n", temp_buf );
    			}
    			fclose(qwerty);
    			remove("dir.txt");
    			//fclose(qwerty);
    		break;

    		default:
    			//printf("%s\n", );
    			bzero(buf,sizeof(buf));
    			n = recvfrom(sockfd,buf,sizeof(buf),0,&serveraddr,&serverlen);
    			printf("In default case\n");
    			printf("%s\n", buf );
    		break;
    	}
    }
    close(sockfd);
}

