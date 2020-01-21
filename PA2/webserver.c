#include <stdio.h>
#include <stdlib.h>
#include <string.h>      
#include <strings.h>     
#include <unistd.h>     
#include <sys/socket.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#define MAXBUF   8192
struct setup{
	int port_number;
	char directory[512];
	char indices[8][64];
	int count;
	char types[16][64];
	char strings[16][64];
	int number_of_types;
} config;

struct http_request{
	int keepAlive;
	char method[256];
	char url[256];
	char version[256];
	size_t request_length;
};

struct http_response{
	char message[4096];
	char type[128];
	char length[128];
};
 
int parseSetup(const char* filename){
	FILE *f;
	size_t len = 0;
	int read   = 0;
	char buffer[256];
	char temp_buffer[256];
	char *line;
	config.number_of_types = 0;

	// Reading through my setup file. 
	printf("Going through the setup file.\n");
	if(!(f = fopen(filename, "r"))){
		printf("Please check your setup/config file and the current directory. Because we cannot open that file rn.\n");
		return -1;
	}

	// Looping through the file, line by line
	while((read = getline(&line,&len, f)) != -1){
		// Put up a mark at the end of a line
		line[read-1] = '\0';
		// Skip unnecessary lines because those are commentary
		if(line[0] == "#" || line[0] == '\0'){
			continue;
		}
		// split it by whitespace
		sscanf(line, "%s %s", buffer, temp_buffer);

		//If first string has following commands
		if(!strcmp(buffer, "Listen")){
			// Cast the second half of the string into int
			// Use it as a port number and validate it
			config.port_number = atoi(temp_buffer);
			if (config.port_number > 65535 || config.port_number < 2) {
		      	printf("Invalid port number.\n");
		      	return -1;
            }
			printf("Port Number: %d\n", config.port_number );
		} else if( !strcmp(buffer, "DocumentRoot")){
			// Get the path to /www
			memcpy(config.directory, &temp_buffer[1], strlen(temp_buffer)-2);
			printf("Directory: %s\n", config.directory );
		} else if ( !strcmp(buffer, "DirectoryIndex")){
			// index.html file
			char *string;
			int index = 0;
			// get the first half
			string = strtok(line, " ");
			// get index.html
			string = strtok(NULL, " ");

			// If there's other than index.html, it's useful
			while(string != NULL){
				printf("Homepage : %s\n", string);
				strcpy(config.indices[index++], string);
				string = strtok(NULL, " ");
			}
			config.count = index;
		} else if (buffer[0] == '.'){
			// Getting the content types
			printf("Read type : %s\n", buffer );
			strcpy(config.types[config.number_of_types], buffer);
			strcpy(config.strings[config.number_of_types++], temp_buffer);
		}
	}
	printf("Done going through the setup file. Now we will initialize the server\n" );
	fclose(f);
	if(line){
		free(line);
	}	
	return 1;
}

void response_f(int fd, struct http_request request, struct http_response response){
	int path_size = strlen(config.directory) + strlen(request.url);
	char path[path_size];
	strcpy(path, config.directory);
	strcat(path, request.url);

	int path_check = 0;
	int method_check = 0;
	FILE *f;

	// if it's a file that we have
	if(!(strcmp(request.url,"/"))){
		int i;
		for (i = 0; i < config.count;i++){
			char temp_path[path_size + strlen(config.indices[i])];
			// path + filename
			strcpy(temp_path, path);
			strcpy(temp_path, config.indices[i]);
			// open index.html
			f = open(temp_path, "rb");
			// if path exists, break
			if(f){
				path_check = 1;
				method_check = 1;
				break;
			}
		}
	}else{
		// If it's not, just open the path of the directory.
		f = fopen(path, "rb");
		if(f){
			path_check = 1;
		}
	}

	// If path does not exist
	if(!path_check){
		// Internal error
		memset(&response.message[0], 0, sizeof(response.message));
		// Adding strings in C. It's excruciating tbh.
		strcpy(response.message, "HTTP/1.1 404 Not Found: ");
		strcat(response.message, request.url);
		strcat(response.message, "\n\n");
		send(fd, response.message, sizeof(response.message), 0);
	}else{
	// If path exists
		char *type;
		int i;

		// Find the correct type from the request.url
		for(i = 0; i<config.number_of_types;i++){
			if(strstr(request.url, config.types[i])){
				type = config.strings[i];
			}
		}

		// Set memory
		memset(&response.message[0], 0, sizeof(response.message));
		memset(&response.type[0], 0, sizeof(response.type));
		memset(&response.length[0], 0, sizeof(response.length));

		// If are not supporting the request type, internal error.
		if(type == NULL && method_check == 0){
			strcpy(response.message, "HTTP/1.1 501 Not Implemented: ");
			strcat(response.message, request.url);
			strcat(response.message, "\n\n");
			printf("INFO: Status of request - Error Encountered, 501\n\n");
			send(fd, response.message, sizeof(response.message), 0);
			return;
		}

		// WE ARE GOOD!
		strcpy(response.message, "HTTP/1.1 200 OK\n");


		if(method_check == 0){
			sprintf(response.type, "Content-Type: %s\n", type);
			printf("Content-Type: %s\n", type);
		} else { 
			// index page
			sprintf(response.type, "Content-Type: %s", "text/html\n");
			printf("Content-Type: text/html\n");
		}


		fseek(f,0,SEEK_END);
		int size = ftell(f);
		sprintf(response.length, "Content-Length: %d\n",size);
		char r[MAXBUF];

		// Create the response and send it back
		strncpy(r,response.message, sizeof(response.message));
		strcat(r, response.type);
		strcat(r, response.length);


		if (request.keepAlive == 1) {
			strcat(r, "Connection: keep-alive\n");
		}
		strcat(r, "\n");

		send(fd, r, strlen(r), 0);

		fseek(f, 0, SEEK_SET);
		size_t read = 0;
		char *fileContents;
		fileContents = malloc(MAXBUF);

		// Read and begin writting the file reqested by the client to the client
		while ((read = fread(fileContents, 1, 8192, f)) > 0) {
			write(fd, fileContents, (int)read);
		}
		send(fd, "\n", 2, 0);
		free(fileContents);
		fclose(f);
	}

	if (path_check == 1) {
		printf("INFO: Status of request - URI Found\n\n");
	} else {
		printf("INFO: Status of request - URI Not Found - Error Encountered, 404\n");
		printf("URI Requested: %s\n\n", request.url);
	}

}


int validation(int fd, struct http_request request, struct http_response response){
	memset(&response.message[0],0,sizeof(response.message));
	if(strcmp(request.method,"GET") != 0){
		strcpy(response.message, "HTTP/1.1 400 Bad Request: Invalid Method: ");
		strcat(response.message, request.method);
		strcat(response.message, "\n\n");
		printf("INFO: Status of request - Error Encountered, 401\n\n");
		send(fd, response.message, sizeof(response.message), 0);
		return 0;
	}
	if (request.url[0] == '\0') {
		strcpy(response.message, "HTTP/1.1 400 Bad Request: Invalid URI: ");
		strcat(response.message, request.url);
		strcat(response.message, "\n\n");
		printf("INFO: Status of request - Error Encountered, 402\n\n");
		send(fd, response.message, sizeof(response.message), 0);
		return 0;
	}
	if (!(strcmp(request.version, "HTTP/1.1") == 0 || strcmp(request.version, "HTTP/1.2") == 0)) {
		strcpy(response.message, "HTTP/1.1 400 Bad Request: Invalid HTTP-Version: ");
		strcat(response.message, request.version);
		strcat(response.message, "\n\n");
		printf("INFO: Status of request - Error Encountered, 403\n\n");
		send(fd, response.message, sizeof(response.message), 0);
		return 0;
	}
	return 1;
}
void Requests(int fd){
 	struct http_response response;
 	struct http_request request;
 	char request_string[MAXBUF];
 	fd_set set;
 	FD_ZERO(&set);
 	FD_SET(fd, &set);
 	// Timer for client socket
 	struct timeval time;
 	time.tv_sec = 10;
 	int current_status;

 	// Initiates the timer and listens
 	// it'll die if it's not alive.
 	while ((current_status = select(fd + 1, &set, NULL, NULL, &time)) > 0){
 		int req_wait = recv(fd, &request_string, 4096,0);
 		if(req_wait != 0){
 			memset(&request.method[0],0,sizeof(request.method));
 			memset(&request.url[0],0,sizeof(request.url));
 			memset(&request.version[0],0,sizeof(request.version));
 			sscanf(request_string, "%s %s %s %*s", request.method, request.url,request.version);
 			int check = validation(fd,request,response);

 			if( check == 1 ){
 				if(strstr(request_string, "Connection: keep-alive")){
 					// Keep it alive
 					time.tv_sec = 10;
 					request.keepAlive = 1;
 				}else{
 					// Kill it
 					time.tv_sec = 0;
 					request.keepAlive = 0;
 				}
 				response_f(fd,request,response);
 			}
 			memset(&request_string[0],0,sizeof(request_string));
 		}
 	}
 	printf("Client connection closed.\n");
	close(fd); 	
}


int ini_Socket(int port_number){
	int listenfd;
	int binary = 1;
	struct sockaddr_in server;
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr, "Can't listen to the specified port %d\n", port_number );
		return -1;
	}else{
		printf("Listening to the port after creating the socket.\n");
	}
	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&binary, sizeof(int)) <0){
		printf("Could not unbind socket.\n");
		return -1;
	}
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(port_number);

	if(bind(listenfd, (struct sockadd*)&server,sizeof(server)) < 0){
		printf("Can't bind server to the socket.\n");
		return -1;
	}
	if(listen(listenfd,1024) < 0){
		printf("Can't start listening.\n");
		return -1;
	}
	return listenfd;
}

int main(int argc, char **argv){
	if (parseSetup("./ws.conf") == -1) {
 		printf("Can't start the server.\n");
 		exit(EXIT_FAILURE);
 	}
 	int listenfd;
 	if((listenfd = ini_Socket(config.port_number)) < 0){
 		exit(EXIT_FAILURE);
 	}else{
 		int con_fd;
 		struct sockaddr_in client;
 		int cli_len = sizeof(struct sockaddr_in);
 		while(1){
 			con_fd = accept(listenfd, (struct sockaddr*)&client,&cli_len);
 			if(con_fd){
 				if(!fork()){
 					Requests(con_fd);
 					exit(0);
 				}
 			}
 			close(con_fd);
 		}
 	}
 	close(listenfd);
 	return 0;
}