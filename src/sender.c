/** @file sender.c
 *  @brief A UDP sender program
 *
 *  This contains the code for the sender
 *  using the UDP file transfer protocol. The
 *  main function parses the command line arguments
 *  and then calls the rsend function to send
 *  the file.
 *
 *  @author Vicky Chen (chen-vv)
 *  @author Eric Omielan (eomielan)
 *  @bug No known bugs.
 */

/* -- Includes -- */

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

#include <pthread.h>
#include <errno.h>
#include "include/udp.h"

/** @brief Sends the first bytesToTransfer bytes of the file
 *         indicated by filename to the receiver at
 *         hostname:hostUDPport.
 *
 *  This function sends the file using the UDP (SOCK_DGRAM)
 *  file transfer protocol. The bytes should be transferred
 *  correctly and efficiently, even if the network drops or
 *  reorders packets. See rrecv for the counterpart function.
 *
 *  @param hostname The name of the receiver host.
 *  @param hostUDPport The port number on the receiver host.
 *  @param filename The name of the file to transfer.
 *  @param bytesToTransfer The number of bytes to transfer.
 *  @return Void.
 */
void rsend(char *hostname,
           unsigned short int hostUDPport,
           char *filename,
           unsigned long long int bytesToTransfer)
{
    // https://www.geeksforgeeks.org/socket-programming-cc/
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    // https://stackoverflow.com/questions/13547721/udp-socket-set-timeout
    struct timeval tv;
    int timeout = DEFAULT_TIMEOUT;
    int retries = 0;
    tv.tv_sec = 0;
    tv.tv_usec = timeout * USEC_PER_MILLISEC;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        perror("timeout");
    }

    // https://www.cs.cmu.edu/~srini/15-441/S10/lectures/r01-sockets.pdf
    struct hostent *host = gethostbyname(hostname);
    if (host == NULL)
    {
        perror("gethostbyname");
        exit(1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(hostUDPport);
    memcpy(&addr.sin_addr.s_addr, host->h_addr, host->h_length);

    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        perror("fopen");
        exit(1);
    }

    unsigned long long bytesSent = 0;
    char data[MAX_BUFFER_SIZE];
    char packet[MAX_BUFFER_SIZE + HEADER_SIZE];
    struct Header header;
    int sequenceNumber = 0;

    while (bytesSent < bytesToTransfer)
    {
        int bytesRead = fread(data, 1, sizeof(data), file);
        if (bytesRead <= 0)
        {
            // https://www.tutorialspoint.com/eof-getc-and-feof-in-c
            if (feof(file))
            {
                // End of file
                break;
            }
            perror("fread");
            exit(1);
        }

        header.sequenceNumber = sequenceNumber;

        memcpy(packet, &header, HEADER_SIZE);
        memcpy(packet + HEADER_SIZE, data, strlen(data) + 1); // Include null terminator

        fprintf(stdout, "Sending packet with sequence number: %d\n", sequenceNumber);

        // https://www.ibm.com/docs/en/zos/3.1.0?topic=functions-sendto-send-data-socket
        int bytesSentThisTime = sendto(sockfd, packet, HEADER_SIZE + strlen(data) + 1, 0, (struct sockaddr *)&addr, sizeof(addr));
        if (bytesSentThisTime < 0)
        {
            perror("sendto");
            exit(1);
        }

        printf("%d bytes sent\n", bytesSentThisTime);

        bytesSent += bytesSentThisTime;
        sequenceNumber++;
        // // TODO: Wait for acknowledgement - set timeout again if needed?
        // socklen_t addrlen = sizeof(addr);
        // int bytesReceived = recvfrom(sockfd, data, MAX_BUFFER_SIZE, 0, (struct sockaddr*) &addr, &addrlen);
        // if (bytesReceived < 0) {
        //     if (errno == EWOULDBLOCK) {
        //         if (retries < MAX_RETRIES) {
        //             timeout *= 2;
        //             printf("No response. Now waiting for %d\n", timeout);
        //             continue;
        //         } else {
        //             printf("No response after %d retries. Exiting.\n", MAX_RETRIES);
        //             exit(1);
        //         }
        //     } else {
        //         perror("recvfrom");
        //         exit(1);
        //     }
        // } else {
        //     printf("Received response: %s\n", data);

        //     bytesSent += bytesSentThisTime;
        //     sequenceNumber++;
        // }
    }

    fclose(file);
    close(sockfd);
}

/** @brief UDP sender entrypoint.
 *
 *  TODO: Implement main & write comments
 *  Mention that this takes in the command line arguments
 *  for invoking the file transfer.
 *
 * @return Should not return
 */
int main(int argc, char **argv)
{
    int hostUDPport;
    unsigned long long int bytesToTransfer;
    char *hostname = NULL;
    char *filename = NULL;

    if (argc != 5)
    {
        fprintf(stderr, "usage: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n\n", argv[0]);
        exit(1);
    }

    hostname = argv[1];
    hostUDPport = (unsigned short int)atoi(argv[2]);
    filename = argv[3];
    bytesToTransfer = atoll(argv[4]);

    fprintf(stdout, "Initializing sender to transfer %s (%lld bytes) to %s:%d\n", filename, bytesToTransfer, hostname, hostUDPport);

    rsend(hostname, hostUDPport, filename, bytesToTransfer);

    return (EXIT_SUCCESS);
}