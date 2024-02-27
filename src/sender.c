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

#define HEADER_SIZE sizeof(struct Header)
#define USEC_PER_MILLISEC 1000

// TODO: comments
struct Header {
    uint32_t sequence_number;
};

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
void rsend(char* hostname, 
            unsigned short int hostUDPport, 
            char* filename, 
            unsigned long long int bytesToTransfer) 
{
    // https://www.geeksforgeeks.org/socket-programming-cc/
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    // https://stackoverflow.com/questions/13547721/udp-socket-set-timeout
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100 * USEC_PER_MILLISEC;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("timeout");
    }

    // https://www.cs.cmu.edu/~srini/15-441/S10/lectures/r01-sockets.pdf
    struct hostent *host = gethostbyname(hostname);
    if (host == NULL) {
        perror("gethostbyname");
        exit(1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(hostUDPport);
    memcpy(&addr.sin_addr.s_addr, host->h_addr, host->h_length);

    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("fopen");
        exit(1);
    }

    unsigned long long bytesSent = 0;
    char buffer[1024];
    char packet[1024 + HEADER_SIZE];
    struct Header header;
    int sequenceNumber = 0;

    while (bytesSent < bytesToTransfer) {
        int bytesRead = fread(buffer, 1, sizeof(buffer), file);
        if (bytesRead <= 0) {
            // https://www.tutorialspoint.com/eof-getc-and-feof-in-c
            if (feof(file)) {
                // End of file
                break;
            }
            perror("fread");
            exit(1);
        }

        header.sequence_number = sequenceNumber;

        memcpy(packet, &header, HEADER_SIZE);
        memcpy(packet + HEADER_SIZE, buffer, strlen(buffer) + 1); // Include null terminator


        // https://www.ibm.com/docs/en/zos/3.1.0?topic=functions-sendto-send-buffer-socket
        int bytesSentThisTime = sendto(sockfd, packet, HEADER_SIZE + strlen(buffer) + 1, 0, (struct sockaddr*) &addr, sizeof(addr));
        if (bytesSentThisTime < 0) {
            perror("sendto");
            exit(1);
        }

        printf("Packet sent with sequence number: %d\n", sequenceNumber);

        bytesSent += bytesSentThisTime;
        sequenceNumber++;
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
int main(int argc, char** argv) {
    // This is a skeleton of a main function.
    // You should implement this function more completely
    // so that one can invoke the file transfer from the
    // command line.
    int hostUDPport;
    unsigned long long int bytesToTransfer;
    char* hostname = NULL;
    char* filename = NULL;

    if (argc != 5) {
        fprintf(stderr, "usage: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n\n", argv[0]);
        exit(1);
    }
    hostUDPport = (unsigned short int) atoi(argv[2]);
    hostname = argv[1];
    filename = argv[3];
    bytesToTransfer = atoll(argv[4]);

    rsend(hostname, hostUDPport, filename, bytesToTransfer);

    return (EXIT_SUCCESS);
}