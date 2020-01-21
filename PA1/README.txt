The program consists of 2 parts:
1. Client
2. Server

To run the programs above, you'll need the following:
make server
./server [port]

make client
./client [server_ip] [server_port]

The following are the possible functionalities that are offered in client:

1. ls 
(list of the files in the server directory)
->writes the current directory status into a file and sends them as packets. After that it get rid of the written file.
->Client receives the file and writes it into file. After that i get rid of the written file.

2. exit
 (exit from client & server)
->client received the exit signal and server&client both exit.

3. delete [filename.type] 
( delete the specified filename from the server)
->Client requests a file to be deleted
->Receive the deletion request-deletes the specified file

4. get [filename.type]
(get the specified file from the server)
->Client requests a file from the server and gets ready to write the data. After writing the data into the file with same name, we close.
->Receive the request and read the packet with given packet size. Send the data packet by packet and close the file.

5. put [filename.type]
(put specified file to the server)

I stripped the user's inputs and used the first letters for my switch cases and the default case will just ignore user. (Send the prompt back to them.)

I could not access to csel instances because my account was tied to my last 2 on-campus jobs. (Cognitive-Science-admin and ECEE-admin.)

I used 3 variables to check/send the data.
-Size
-Bits delivered so far
-data