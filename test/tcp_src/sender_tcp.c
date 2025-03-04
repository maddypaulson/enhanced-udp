/**
 * @file sender_tcp.c
 * @brief Functions for sending data over TCP protocol.
 * 
 * This file contains the implementation of functions
 * responsible for sending data over the TCP protocol. 
 * It includes a key function tsend which handles the
 * connection
 * 
 * @author Maddy Paulson (maddypaulson)
 * @author Leo Kamino (LeonardoKamino)
 * @bug No known bugs.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../../src/includes/packet_header.h"
#include "../../src/includes/test_output.h"


#define BUFFER_SIZE 1024
#define LINK_CAPACITY 20971520 
 
/**
 * @brief Sends a file over a TCP connection.
 * 
 * @param hostname The receiver's hostname or IP address.
 * @param hostPort The receiver's TCP port.
 * @param filename The path to the file to be sent.
 * @param bytesToTransfer The number of bytes to transfer from the file.
 */
void tsend(char* hostname, unsigned short int hostPort, char* filename, unsigned long long int bytesToTransfer) {
    int sockfd;
    struct sockaddr_in destAddr;
    char buffer[BUFFER_SIZE];
    ssize_t readBytes;
    FILE *file;
    unsigned long long int totalBytesSent = 0;

    // Resolve the hostname to an IP address
    struct addrinfo hints, *servinfo;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status;
    printf("Hostname: %s\n", hostname);
    if((status = getaddrinfo(hostname, NULL, &hints, &servinfo)) != 0){
        perror("Address translation failed.");
        exit(EXIT_FAILURE);
    }


    // Create TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(hostPort);
    memcpy(&destAddr.sin_addr, &((struct sockaddr_in *)servinfo->ai_addr)->sin_addr, sizeof(struct in_addr));

    freeaddrinfo(servinfo);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&destAddr, sizeof(destAddr)) < 0) {
        perror("ERROR connecting");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Open file for reading
    file = fopen(filename, "rb");
    if (file == NULL) {
        perror("ERROR opening file");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Start timing for bandwidth calculation
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Send the file data
    while (totalBytesSent < bytesToTransfer && (readBytes = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        ssize_t sentBytes = sendto(sockfd, buffer, readBytes, 0, NULL, 0);
        if (sentBytes < 0) {
            perror("ERROR writing to socket");
            break;
        }
        totalBytesSent += sentBytes;
    }

    /*
    * Calculate and print bandwidth utilization
    */

    gettimeofday(&end, NULL);
    
    displayPerformance(&start, &end, totalBytesSent);


    fclose(file);
    close(sockfd);
}

/**
 * @brief Entry point for the TCP file sender program.
 * 
 * This function parses command line arguments and initiates the file transfer process
 * by calling the tsend function. The program expects exactly four arguments:
 * the receiver's hostname, the receiver's TCP port, the filename to transfer,
 * and the number of bytes to transfer.
 * 
 * @param argc The number of command line arguments.
 * @param argv The array of command line arguments.
 * @return int EXIT_SUCCESS on successful completion or EXIT_FAILURE on error.
 */
int main(int argc, char** argv) {
    if (argc != 5) {
        fprintf(stderr, "usage: %s receiver_hostname receiver_TCPport filename_to_xfer bytes_to_xfer\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    unsigned short int hostPort = (unsigned short int)atoi(argv[2]);
    char* hostname = argv[1];
    unsigned long long int bytesToTransfer = atoll(argv[4]);
    char* filename = argv[3];

    tsend(hostname, hostPort, filename, bytesToTransfer);

    return EXIT_SUCCESS;
}
