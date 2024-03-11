/**
 * client.c
 *
 * @author Rim Tyouss
 *
 * USD COMP 375: Computer Networks
 * Project 1
 *
 * TODO: COMP 375 - Project #1: Reverse Engineering a Network Application
 */

#define _XOPEN_SOURCE 600

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <argp.h>

#define BUFF_SIZE 1024

long prompt();
int connect_to_host(char *hostname, char *port);
void main_loop();
void sensor_data(char *selection);
void send_request(int server_fd, char* buff);
void rcv_response(int server_fd, char* buff);
//void parse(char *buff, char *parse_buff[], int size);

int main() {
	printf("WELCOME TO THE COMP375 SENSOR NETWORK\n\n");

	main_loop();
	return 0;
}

/**
 * Loop to keep asking user what they want to do and calling the appropriate
 * function to handle the selection.
 */
void main_loop() {

	while (true) {

		long selection = prompt();

		switch (selection) {
			case 1:
				// TODO: Handle case one by calling a function you write
			// TODO: add cases for other menu options
				sensor_data("AIR TEMPERATURE");
				break;
			case 2:
				sensor_data("RELATIVE HUMIDITY");
				break;
			case 3:
				sensor_data("WIND SPEED");
				break;
			case 4:
				printf("GOODBYE!\n");
				exit(0);
				break;
			default:
				fprintf(stderr, " *** Invalid selection.\n");
				break;
		}
	}

}

/** 
 * Print command prompt to user and obtain user input.
 *
 * @return The user's desired selection, or -1 if invalid selection.
 */
long prompt() {
	// TODO: add printfs to print out the options
	printf("\nWhich sensor would you like to read:\n\n");
    printf("\t(1) Air temperature\n");
    printf("\t(2) Relative humidity\n");
    printf("\t(3) Wind speed\n");
    printf("\t(4) Quit Program\n\n");
    printf("Selection: ");

	// Read in a value from standard input
	char input[10];
	memset(input, 0, 10); // set all characters in input to '\0' (i.e. nul)
	char *read_str = fgets(input, 10, stdin);

	// Check if EOF or an error, exiting the program in both cases.
	if (read_str == NULL) {
		if (feof(stdin)) {
			exit(0);
		}
		else if (ferror(stdin)) {
			perror("fgets");
			exit(1);
		}
	}

	// get rid of newline, if there is one
	char *new_line = strchr(input, '\n');
	if (new_line != NULL) new_line[0] = '\0';

	// convert string to a long int
	char *end;
	long selection = strtol(input, &end, 10);

	if (end == input || *end != '\0') {
		selection = -1;
	}

	return selection;
}

/**
 * Socket implementation of connecting to a host at a specific port.
 *
 * @param hostname The name of the host to connect to (e.g. "foo.sandiego.edu")
 * @param port The port number to connect to
 * @return File descriptor of new socket to use.
 */
int connect_to_host(char *hostname, char *port) {
	// Step 1: fill in the address info in preparation for setting 
	//   up the socket
	int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;  // will point to the results

	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_INET;       // Use IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

	// get ready to connect
	if ((status = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

	// Step 2: Make a call to socket
	int fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (fd == -1) {
		perror("socket");
		exit(1);
	}

	// Step 3: connect!
	if (connect(fd, servinfo->ai_addr, servinfo->ai_addrlen) != 0) {
		perror("connect");
		exit(1);
	}

	freeaddrinfo(servinfo); // free's the memory allocated by getaddrinfo

	return fd;
}

/**
 * Sends a request message to the relevant server with a socket
 * @param server_fd The file descriptor of the server socket to send the request to.
 * @param buff A buffer with the request message to send.
 * 
 */

void send_request(int server_fd, char* buff) {

    if(send(server_fd,buff,strlen(buff),0)==-1)
	{
		perror("send");
		exit(1);
	}
}

/**
 * Receives a response message from the server identified by the socket file descriptor.
 *
 * @param server_fd The file descriptor of the server socket to receive the response from.
 * @param buff A buffer to store the received response message.
 * 
 */

void rcv_response(int server_fd, char *buff) {

int numbytes = recv(server_fd,buff,BUFF_SIZE, 0);
if (numbytes == -1)
	{
		perror("recv");
        exit(1);
    }
	buff[numbytes] = '\0';
	//printf("Debug -- received response: \n%s\n", buff);

}

/**
 * Requests air temperature, relative humidity or wind speed from USD's sensor based on the user's selection. It also parses the server's response,
 * and shows the result.
 *
 * @param selection The name of the sensor data to request chosen by the user (AIR TEMPERATURE, RELATIVE HUMIDITY, WIND SPEED).
 * 
 */

void sensor_data(char *selection) // i have to parse server response because if not "Failed to parse server response.
{
	char buff[BUFF_SIZE];

	int fd = connect_to_host("hopper.sandiego.edu", "7030");

	send_request(fd,"AUTH password123\n"); // tshark 17 AUTH password123\n
	rcv_response(fd,buff); // receive the response in the buffer
	close(fd);
	

	//so we can extract the server, port, and password frOm the buffer
	char server[100], port[6], password[100]; 
	if (sscanf(buff, "CONNECT %s %s %s", server, port, password) < 3) { // if less than the three items were read
        printf("parse failed.\n");
        exit(1);
    }

	fd = connect_to_host(server,port); //connect to server and port

	send_request(fd,"AUTH sensorpass321\n"); //  CONNECT weatherstation.sandiego.edu 7054 sensorpass321\n
    rcv_response(fd,buff); 

	// requesting the data from the sensor
	char req[BUFF_SIZE];
	snprintf(req, sizeof(req), "%s\n", selection);
	send_request(fd, req);
    rcv_response(fd, buff);


	//for the time

	long time;
    char value[10], unit[10];
    if (sscanf(buff, "%lu %s %s", &time, value, unit) == 3) { //lu:long integer
        time_t time_string = (time_t)time;
        printf("\nThe last %s reading was %s %s, taken at %s", selection, value, unit, ctime(&time_string));
    } else {
        printf("parse failed.\n");
    }

	send_request(fd,"CLOSE\n");
	rcv_response(fd,buff);
	memset(buff,0,BUFF_SIZE);
	close(fd);
}

/*
void parse(char *buff, char *parse_buff[], int size) {
     for (int i = 0; i < size; i++) {
        if (sscanf(buff, "%s", buff) > 0) {
            parse_buff[i] = strdup(buff); 
            buff += strlen(parse_buff[i]) + 1;
        } else {
            break; 
        }
    }
}*/




