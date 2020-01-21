#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#define BUF 1024
#define DATABUFSIZE 131072


// Cache struct
// Has the : Page data, url, ip, page size, 
// last time it got called, and pointer to next page in the cache

struct cache{
	char page_data[DATABUFSIZE];
	char url[BUF];
	char ip[BUF];
	int p_size;
	time_t lastCall;
	struct cache *next;
};

// Declaring the cache struct for the program
typedef struct cache cache;
cache* head = NULL;
int cache_life;


// Function to add page into the cache
// Basically creating cache struct and 
// The new page becomes the head
void add_to_cache(char *data, char *url, char *ip, int size){
	cache *page = (cache *)malloc(sizeof(page)*DATABUFSIZE);
	page->next = head;
	strcpy(page->page_data,data);
	strcpy(page->url,url);
	strcpy(page->ip,ip);
	page->p_size = size;
	page->lastCall = time(NULL);
	head = page;
}

// Validating the hostname and ip address 
// Using gethostbyname
int ip_fun(char* host_name, char*ip){
	struct hostent *host_in_ip;
	struct in_addr **addr_list;
	int i;
	// puts(host_name);
	// puts(gethostbyname(host_name));

	//Getting the host entity struct using the host_name
	host_in_ip = gethostbyname(host_name);
	if(host_in_ip == NULL){
		herror("error in ip_fun and gethostbyname");
		return 1;
	}	
	addr_list = (struct in_addr **) host_in_ip->h_addr_list;
	for(i=0;addr_list[i] != NULL;i++){
		// The inet_ntoa() function converts the Internet host address in,
		// given in network byte order, to a string in IPv4 dotted-decimal notation.
		// The string is returned in a statically allocated buffer, which subsequent calls will overwrite.
		strcpy(ip,inet_ntoa(*addr_list[i]));
		return 0;
	}
	return 1;
}


// Finding page from the cache
cache* find_page_from_cache(char *url, char*ip){
	cache *page = NULL;
	if(head != NULL){
		for(page = head; page!= NULL; page = page->next){
			if(strcmp(page->url,url) == 0){
				time_t expiration = difftime(time(NULL),page->lastCall);
				if(expiration >= cache_life){
					page->lastCall = time(NULL);
					return NULL;
				}
				return page;
			}
		}
	}
	else{
		printf("Cache empty\n");
	}
	return NULL;
}


// Getting the domain from the url that we use daily
void host_name_f(char* request, char* host){
	// sscanf - read formatted data from a string
	sscanf(request, "http://%511[^/\n]",host);
}


// Check if it's a valid ip address
int ip_validation(char *ip){
	struct sockaddr_in yo;
	int result = inet_pton(AF_INET, ip, &(yo.sin_addr));
	// puts(ip);
	return result != 0;
	// if(result == 1){
	// 	return 1;
	// }
	// return 0;
}


// 
int http_header(int s_sock, char *header_buffer, int header_size){
	int i = 0;
	char c = '\0';
	int receiving;
	// puts(header_size);
	// loop until we get the header
	while(i < (header_size-1) && (c != '\n')){
		// receive data from the server socket
		receiving = recv(s_sock,&c,1,0);
		// If receiving is successful
		if(receiving >0){
			// If there is a line break
			if(c == '\r'){
				// Just peek the server's message
				receiving = recv(s_sock,&c,1,MSG_PEEK);
				// If peeked message is not empty and new line definer
				// Read that new line defining character
				if((receiving >0) && (c == '\n')){
					recv(s_sock,&c,1,0);
				}
				// If there's no new line definer, set up the new line
				// defining variable again
				else{
					c = '\n';
				}
			}
			// Add the received data into the buffer 
			// that we created
			header_buffer[i] = c;
			// move forward
			i++;
		}
		else{
			// if there's nothing to receive
			// we break here
			c = '\n';
		}
	}
	// Terminator of the character string
	header_buffer[i] = '\0';
	// puts("in http_header and printing i");
	// puts(i);

	// return the index that we traversed until
	return i;
}

int get_server_response(int s_sock, char*buffer){
	char body[DATABUFSIZE];
	int length = 0;
	unsigned int offset = 0;
	while(1){
		// Get size of the header
		int size = http_header(s_sock,body,DATABUFSIZE);
		// Nothing, then break
		if(size <= 0){
			puts("something wrong with http header in get_server_response");
			return -1;
		}
		// allocate more data
		memcpy((buffer+offset),body,size);
		// Increment the current offset in the data buffer
		offset+= size;
		// If body is small then break
		if(strlen(body) == 1){
			break;
		}
		// If there's content length part then get the content length
		if(strncmp(body,"Content-Length",strlen("Content-Length")) == 0){
			char sk1[1024];
			sscanf(body,"%*s,%s", sk1);
			length = atoi(sk1);
		}
	}

	// allocate the content memory 
	char* body_buffer = malloc((length*sizeof(char))+3);
	int i;

	// traverse through content 
	for(i = 0; i< length; i++){
		char c;
		// receive data from server socket
		int receiving = recv(s_sock,&c,1,0);
		// if receiving is empty
		if(receiving <= 0){
			return -1;
		}
		// allocate that received data in the buffer
		body_buffer[i]=c;
	}

	// New lines + null terminator
	body_buffer[i+1] = '\r';
	body_buffer[i+2] = '\n';
	body_buffer[i+3] = '\0';
	//printf(body_buffer);

	//allocate the data that we've read into the buffer
	memcpy((buffer+offset),body_buffer,(length+3));
	free(body_buffer);

	//current offset + content length + 4 
	return (offset+i+4);
}

// Handling errors like last assignment
int error_handle(int c_sock, int status, char*method, char*URI, char*version, char*host_name){
	char header[BUF];
	char data[DATABUFSIZE];
	int length = 0;

	memset(&header,0,BUF);
	memset(&data,0,DATABUFSIZE);

	if(strcmp(method, "GET") != 0){
		status = 501;
	}else if((strcmp(version, "HTTP/1.1") != 0) && (strcmp(version, "HTTP/1.0") != 0)){
		status = 400;
	}else{
		FILE *f = fopen("blocked.txt" , "r");
		char block_host[BUF];
		char block_URI[BUF];
		char block_IP[BUF];
		char host[BUF];
		if(f!= NULL){
			while(!feof(f)){
				fgets(block_host, BUF, f);
				if(block_host[strlen(block_host)-1] == '\n'){
					block_host[strlen(block_host)-1] = '\0';
				}
				if(ip_validation(block_host)){
					ip_fun(host_name,block_IP);
					//puts("in error handle function calling error handler");
					strcpy(URI,block_IP);
					strcpy(block_URI,block_host);
				}else{
					snprintf(block_URI, BUF, "http://%s/", block_host);
				}
				if(strcmp(URI,block_URI) == 0){
					status = 403;
					break;
				}
			}
		}
	}

	    switch(status) {
        case 0:
            return 0;
        case 400:
            snprintf(data, BUF, "<html><body>400 Bad Request Reason: "
                     "Invalid Version:%s</body></html>\r\n\r\n", version);
            
            //header
            length += snprintf(header, BUF, "HTTP/1.1 400 Bad Request\r\n");
            length += snprintf(header+length, BUF-length, "Content-Type: text/html\r\n");
            length += snprintf(header+length, BUF-length, "Content-Length: %lu\r\n\r\n", strlen(data));
            send(c_sock, header, strlen(header), 0);
            write(c_sock, data, strlen(data));
            return -1;

        case 403:
            snprintf(data, BUF, "<html><body>ERROR 403 Forbidden: "
                     "%s</body></html>\r\n\r\n", URI);
            
            //header
            length += snprintf(header, BUF, "HTTP/1.1 ERROR 403 Forbidden\r\n");
            length += snprintf(header+length, BUF-length, "Content-Type: text/html\r\n");
            length += snprintf(header+length, BUF-length, "Content-Length: %lu\r\n\r\n", strlen(data));
            send(c_sock, header, strlen(header), 0);
            write(c_sock, data, strlen(data));
            return -1;
        case 501:
            snprintf(data, BUF, "<html><body>501 Not Implemented "
                     "Method: %s</body></html>\r\n\r\n", method);
            length += snprintf(header, BUF, "HTTP/1.1 501 Not Implemented\r\n");
            length += snprintf(header+length, BUF-length, "Content-Type: text/html\r\n");
            length += snprintf(header+length, BUF-length, "Content-Length: %lu\r\n\r\n", strlen(data));
            send(c_sock, header, strlen(header), 0);
            write(c_sock, data, strlen(data));
            return -1;
        default:
        	puts("In default error");
            length += snprintf(header, BUF, "HTTP/1.1 500 Internal Server Error\r\n");
            length += snprintf(header+length, BUF-length, "Content-Type: text/html\r\n");
            length += snprintf(header+length, BUF-length, "Content-Length: %lu\r\n\r\n", strlen(data));
            snprintf(data, BUF, "<html><body>500 Internal Server Error: "
                     "Cannot allocate memory</body></html>\r\n\r\n");
            send(c_sock, header, strlen(header), 0);
            
            write(c_sock, data, strlen(data));
            return -1;
    }
}



// Handling client request
void client_req(int c_sock,char* message, char* URI,char* host_name){
	char server_message[DATABUFSIZE];
	memset(&server_message,0,DATABUFSIZE);

	int s_sock;
	struct sockaddr_in server;

	// Creating socket for the client
	s_sock = socket(AF_INET, SOCK_STREAM,0);
	if(s_sock == -1){
		printf("Can't initiate a socket to server\n" );
	}

	// set sock options to listening
	int sock_option = 1;
	setsockopt(s_sock, SOL_SOCKET, SO_REUSEADDR, &sock_option,sizeof(sock_option));

	// Create host entity, validate and get the addresses
	struct  hostent* host_ent;
	host_ent = gethostbyname(host_name);
	char req_ip[1024];
	ip_fun(host_name,req_ip);

	// If the host_ent is NULL there is no host under the given domain
	if(host_ent == NULL){
		printf("gethostbyname error: %s\n", strerror(h_errno));
	}

	// Server set up
	server.sin_family = AF_INET;
	memcpy(&server.sin_addr, host_ent->h_addr_list[0],host_ent->h_length);
	server.sin_port = htons(80);
	socklen_t server_size = sizeof(server);


	// Connect server socket to socket address
	int s_connection = connect(s_sock, (struct sockaddr *) &server, server_size);
	if(s_connection < 0){
		perror("Can't to connect to host");
        close(c_sock);
        return;
	}

	// message length
	int message_len = strlen(message);
	snprintf(message+message_len, DATABUFSIZE, "\r\n\r\n");
	//puts("in client_req");
	//puts(message);

	// Send message to server socket
	// forwarding the message
	send(s_sock, message, strlen(message), 0);

	// Get server's response and get the offset that it returns
	int receiving = get_server_response(s_sock,server_message);
	//puts(receiving);

	// If it's 0, then there's no data to be received
	// <0, then there's error
	if(receiving == 0){
		printf("Host disconnected: %d\n", s_sock);
        fflush(stdout);
	}
	else if(receiving < 0){
		fprintf(stderr, "recv error: \n");
	}

	// adding that requested page to cache
	add_to_cache(server_message,URI,req_ip, receiving);
	//printf(server_message );
	//printf("Sending message to client \n");

	// Forwarding the server's response to the client socket
	send(c_sock, server_message, receiving, 0);
	//printf("Successfully sent message to client\n");
	close(s_sock);
}


int main(int argc, char* argv[]){
	// Initializations
	int socket_desc, c_sock, receiving;
	struct sockaddr_in server;

	int status = 0;
	pid_t process_id;
	cache *req_page;

	// Check the number of arguments
	if(argc < 3){
		printf("Please use the script with: ./webproxy <port> <cache-timeout>\n");
        return 1;
	}

	// Cache life is the 3rd argument
	cache_life = atoi(argv[2]);

	// Create the socket with SOCK_STREAM
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_desc < 0){
		 printf("Did not / could not create a socket");
	}

	// Set up the socket options as re-usable
	int sock_option = 1;
	setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &sock_option, sizeof(sock_option));

	// Initialize the server
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(atoi(argv[1]));

	// Bind the socket with server
	if(bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0){
		perror("Could not bind in main.");
        exit(1);
	}

	// Listen to the server socket
	if(listen(socket_desc,10) < 0){
		perror("Could not start listening in main");
        exit(1);	
    }
    while(1){

    	// Client sockets 
    	struct sockaddr_in client;
    	int c = sizeof(struct sockaddr_in);

    	char c_message[DATABUFSIZE];
    	char req_method[1024];
    	char req_URI[1024];
    	char req_version[1024];
    	char host_name[1024];

    	// for listening sockets
    	c_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);

    	// error message
    	if(c_sock < 0){
    		perror("accept failed in main");
            exit(1);
    	}

    	// fork if there's a new client
    	if((process_id = fork()) == 0){
    		// close the proxy socket since we already established 
    		// connection with this client
    		close(socket_desc);

    		//receive data from the client
    		while((receiving = recv(c_sock , c_message , DATABUFSIZE , 0)) > 0){
    			memset(&req_method,0,1024);
    			memset(&req_URI,0,1024);
    			memset(&req_version,0,1024);

    			// extract the data from the client's message
    			sscanf(c_message,"%s %s %s", req_method, req_URI, req_version);

    			//puts(req_method);puts(req_version);puts(req_URI);

    			// calling the hostname function
    			host_name_f(req_URI, host_name);
    			char req_IP[1024];
    			// go to error handle, just to check everything
    			status = error_handle(c_sock, 0 , req_method, req_URI,req_version, host_name);

    			// if status is 0 with no errors
    			if(status == 0){
    				// get the ip
    				ip_fun(host_name, req_IP);
    				// try to find it from the cache
    				req_page = find_page_from_cache(req_URI, req_IP);
    				// If there's nothing in cache, then request it from the server
    				if(req_page == NULL){
    					printf("Requesting page from server...\n");
    					client_req(c_sock,c_message, req_URI, host_name);
    					printf("Successfully requested page from server...\n");
    				}else{
    				// If there's data in the cache that we can use,
    				// then use that
    					printf("Requesting page from cache...\n");
    					strcat(req_page->page_data, "\r\n\r\n");
    					// Send that data back from the proxy to client without even
    					// requesting it from the server
    					send(c_sock, req_page->page_data,req_page->p_size,0);
    				}
    			}
    			// Empty the client message after every iteration and 
    			// Go back to the beginning of the loop and do the same thing
    			memset(&c_message, 0, DATABUFSIZE);
    		}

    		// if there's nothing receiving then flush everything in the memory
    		if(receiving == 0){
    			puts("flushing out");
    			fflush(stdout);
    		}
    		// if the receiving is failed then error
    		else if(receiving < 0){
    			perror("recv failed");
    		}

    		// close the client socket in the child
    		close(c_sock);
    		exit(0);
    	}
    }
    // Close the proxy socket in the main function
    close(socket_desc);
    return 0;
}