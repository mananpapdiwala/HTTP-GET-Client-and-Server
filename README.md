Computer Networks P538
Manan Papdiwala

Along with this readme I have uplodaed a zip file which contains the 3 directories:
1) TCP
2) UDP
3) Multithreaded_Server

Note: Please do not change the position of any file since it will cause errors in makefile.

For Part-I: Webserver please use the directory TCP/Server
The server.cpp contains the code for it. It takes 1 input, the port number. If less than 1 or more than 1 parameter are passed the program prints an error message and closes the program.
It uses TCP socket and supports both persistent and non-persistent connection using the Connection attribute in the HTTP header.
The BUFFSIZE macro is used to define the length of the buffer. My current implmenetation has used this valus as 10000000 but for much more larger file we can increase the buffer size.
When I try to run request the file using web browser the brwoser uses persistent connection as defualt even though if we request only one file. In this case my server and browser goes into deadlock since browser is waiting for server and server is waiting for browser to close connection. Hence I have used timeout
for this purpose. Current, timeout is 5 secs. This timer gets renewed after every request. If client do not reuqest for 5 secs then server closes the connection. Hence when you request file using the browser you will find the
application to be slower but by usng client implementation in Part-II the process is faster.
The defualt queue size for server to listen request is 10.
This folder has a makefile. The commands to run are:

make - compiles the file and generates the server output file
make run PORT={port number} - runs the server file with port number passed as parameter.
E.g make run PORT=20000
make clean - deletes the file.

Part-II: Web Client uses the directory TCP/Client
The client.cpp contains the code for it. It takes atleast 4 inputs. Less than 4 parameters will cause the program to print error message and exit. It uses TCp socket and supports both non-persistent connection and persistent connection.
The first parameter is the server IP.
The second parameter is the server port.
Third parameter is the conection type. Use 1 for persistent connection. Use for 0 for non persistent connection. Any other value will print error message and close the program.
All the parameters after connection type are file names.
E.g. ./client 129.79.242.30 20000 0 a.txt index.html
In non persistent mode the client creates separate socket for each file and uses the bConnection field as closed in HTTP header.
In persistent connection client creates only one socket and sends requests for all the files with the same socket.
Except for the last file all the file requests uses connection field with value keep-alive. The last file uses closed value for the connection field in the header.
The default size of the buffer is 100000. But this can be changed by updating the macro.

The directory also contains makefile. The commands to run the makefile are:
make - compiles the file and generates the client output file.
make run ${HOST} ${PORT} ${CONNECTION_TYPE} ${FILE_NAME} - runs the client file with parameters passed in the make command.
E.g. make run HOST=129.79.242.30 PORT=20000 CONNECTION_TYPE=0 FILE_NAME=a.txt
Note: Through the make command we can only pass 4 parameters. For passing more parameters or easier way to the run the output file will be:
./client 129.79.242.30 20000 0 a.txt
./client 129.79.242.30 20000 0 a.txt index.html

Use the above command only after running make (1) command to first compile the file and generate the client output file.

PART-II: Multi-threaded Web Server uses the directory Multithreaded_server
The multithreaded_server.cpp contains the code for it. The implementation is almost similar to non threaded server. The only difference lies is the main thread listens on the port passed as parameter.
As soon as the request arrives a new socket is created and a new thread is spawn to handle the request on the new socket. The old thread continues listening to the listening port.
The newly created thread runs in parallel and proivdes service to the client. Once it finishes its task it closes the newly created and assigned socket and destroies itself.

The directory also contain the makefile and commands to run the multithreaded server is same as running the non-multithreaded part.
The Multithreaded server supports both persistent and non-persistent connection similar to the original server.

Part-III: Connection-less, Unreliable client and server uses the directory UDP
It further contains sub-directories Client and Server. The Client directory contains the client.cpp file which has the implementation for UDP client.
The server directory contains the server.cpp file which has the implementation for UDP server.
The implementation for both client and server is using the UDP socket. Both the directories has the makefile.
The command to run the makefile for UDP server is same as running the TCP server.
The command to run the makefile for UDP client has two differences. First, it do not accept the connection type parameter and second since this is connectionless implementation only one file name can be passed in the parameter.
E.g. make run HOST=129.79.242.30 PORT=20000 FILE_NAME=a.txt
Or ./client 129.79.242.30 20000 a.txt
Please make sure to use the above command only after compiling the source code with make command.
Again the default size for buffer is 10000000 for server but 10000 for client.
The server appends "EOF" string to indicate end of data or to indicate that last byte has been transmitted.
Please do not use EOF string as data in the file.
The client counts the bytes it receives including the EOF signal and header and prints it on the stdout.

Important key notes for UDP part of the write up:
1) I intially tried to send all the data from the requested file into the socket but that gave me an error stating message size is too big.
For that purpose I had to fragment my data on the application layer itself before sending into the socket. Currently my server reads the data from the file into a big buffer.
Then part of the buffer is send within each loop until all the data has been send.

2) Also, on the client side there arises time when there is pakcet loss and the I do not receive EOF signal and in such case my client goes into deadlock state since recvfrom() is a blocking
function.
So I referred some online help and found and article on stackoverflow which I have referenced below. The article mentioned the use of automatic timeout after a fixed period of time.
So I my recvfrom function do not receive any data during that timeout period than it returns back.

Note: The client prints the file content on the stdout.

Part-IV: Writeup

1) this is table I obtained by running my client for input ranging from 1 to 10 1MB file using both persistent and non persistent connection
Number of files NPC(in s) PC(s)
1				0.062		0.073
2				1.74		0.168
3				0.264		0.195
4				0.319		0.31
5				0.503		0.401
6				0.532		0.527
7				0.554		0.559
8				0.712		0.684
9				0.789		0.77
10				0.875		0.813

Note: the client and sevrer were run on same machine and hence the times are relatively low.

As we can from the table the time increases with increase in the number of files. The increase is almost linear.
The time for NPC (Non Persistent Connection) usually took more time than persistent Connection. The amount may not be that significant. This may be due to overhead in creating new socket which invloves hand shaking
for each connection and also the overhead to close the connection. Since my client is working in sequentially even though I may use 10 different connections, as long as they are sequential I will not be
able to get the advantages of using NPC which is usually achieved by using parallel connections.
The taken to service grows more linearly for NPC due to the overhead to connection open and close in each connection. But for PC it is slightly less since the connection is opened and closed only once.
But the time taken for transferring file dominates and hence we may not be able to visualize significant changes but with increase in Number of files the PC improves a bit due to time saved in 
not performing the overhead work.
The time taken to transfer 10 files for PC is 0.813s.
It transferred almost 10MB of data which is almost 10485760 bytes.
If we assume the packet contains 1460 bytes of data then we may have send atleast 7182 packets.
If we divide the total time by total number of packets we may get a good approximation of Round Trip Time which is 0.113ms

2) the table multithreaded server
Number of files NPC(in s) PC(in s)
1				0.043		0.078
2				0.195		0.17
3				0.205		0.27
4				0.34		0.32
5				0.41		0.441
6				0.518		0.567
7				0.619		0.61
8				0.69		0.73
9				0.81		0.792
10				0.912		0.887

my initial assumption was that the multithreaded server would be faster and indeed it is faster but since my client is working sequentially it becomes the bottleneck.
if tried to get the 10 files from 10 different clients then I would have achieved much optimization but since my one client is sequentially requesting 10 files
even though my server creates a parallel thread to service the request, I would not get the benefits of parallelism due to my sequential client
The multithreaded server took a significantly minute more time due to overhead of spa2ning new threads and hence the overhead. If my I requested the files from 10 different clients
than the overhead of spwaning new thread would have been worth it.

I did a small experiment where I requested 5 1 MB files from 5 different clients. The time calculated among 5 clients came out to be 0.11 which is way better
when I did the same experiment with non multithreaded server.
In non multithreaded server the other clients had to wait while server serviced one client.
The avergae for my non sequential turn out to be around 0.59 s
hence by multithreaded server I achieved almost 4x speed up

3)
the time taken to request and receive the 1 MB file for UDP was 0.125 s
Theoretically UDP should be faster than TCP but in my implementation UDP was slower and bascially due to two reasons.
1) I had to fragment my data on application layer and send it in loop. This caused more time for server to process the data.
2) I had to use timeout on the client side and if the EOF signal is received then the timeout would not add to the total time. But the packet loss causes to lose EOF signal than
the timeout time adds to the total time.

These are the main reasons for my implementation to be much slower than TCP.
When I tried sending 1 MB file I created two isntance using ssh to the burrow machine. One ran the client and another ran the server. But the virtual machine were on the same machine and had smae IP address.
There was some packet loss in receiving the file. I should have received 1048576 bytes but I received 1043176 and thus loss was around 5400 bytes. 

I even tried comparing my original file with my received file on textcompare.com to see the changes.
I hope when I trasnfer file between two different hosts than I would receive much more significant loss.

Note: If given huge file to send by server the code can give segmentation fault unless the BUFFSIZE is not increased accordingly.
References:

https://www.geeksforgeeks.org/socket-programming-cc/

http://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html

https://www.abc.se/~m6695/udp.html

https://stackoverflow.com/questions/13547721/udp-socket-set-timeout
