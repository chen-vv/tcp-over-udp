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
#include <sys/stat.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#include <pthread.h>
#include <errno.h>
#include "include/udp.h"

/**
 * @brief Get the File Size object
 *
 * @param filename
 * @return long long The size of the file in bytes
 */
long long getFileSize(const char *filename)
{
    struct stat st;
    if (stat(filename, &st) == 0)
        return st.st_size;
    else
        return -1;
}

void receive_syn(int sockfd, struct sockaddr_in *addr, socklen_t addrlen)
{
    printf("Waiting for SYN packet\n");

    // set socket to non-blocking mode
    int flags = fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, flags) == -1)
    {
        perror("fcntl - F_SETFL");
        exit(1);
    }

    while (1)
    {
        char syn;
        ssize_t bytes_received = recvfrom(sockfd, &syn, sizeof(char), MSG_DONTWAIT, (struct sockaddr *)addr, &addrlen);
        if (bytes_received > 0)
        {
            printf("Received SYN packet\n");
            break;
        }
        else if (bytes_received == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // no syn yet
                continue;
            }
            else
            {
                perror("recvfrom");
                exit(1);
            }
        }
    }

    // revert socket to blocking mode
    flags &= ~O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, flags) == -1)
    {
        perror("fcntl - F_SETFL");
        exit(EXIT_FAILURE);
    }

    // send syn-ack
    char syn = 0;
    ssize_t size = sendto(sockfd, &syn, sizeof(char), 0, (struct sockaddr *)addr, addrlen);
    if (size == -1)
    {
        perror("sendto");
        exit(1);
    }
}

// TODO: comments
void checkAck(struct sockaddr_in addr, int sockfd, int *sequenceNumber,
              unsigned long long *bytesSent, int bytesSentThisTime, int *timeout,
              int *retries, struct timeval *tv)
{
    socklen_t addrlen = sizeof(addr);
    char ack[MAX_ACK_SIZE];

    int bytesReceived = recvfrom(sockfd, ack, MAX_ACK_SIZE, 0, (struct sockaddr *)&addr, &addrlen);
    if (bytesReceived < 0)
    {
        if (errno == EWOULDBLOCK)
        {
            if (*retries < MAX_RETRIES)
            {
                *timeout *= 2;
                (*retries)++;

                return;
            }
            else
            {
                perror("max timeout reached");
                exit(1);
            }
        }
        else
        {
            perror("recvfrom");
            exit(1);
        }
    }

    uint32_t receivedSequenceNumber;
    memcpy(&receivedSequenceNumber, ack, MAX_ACK_SIZE);

    if (receivedSequenceNumber != *sequenceNumber)
    {
        *timeout *= 2;
        (*retries)++;
        return;
    }

    *bytesSent += (bytesSentThisTime - HEADER_SIZE);
    (*sequenceNumber)++;
    *retries = 0;
    *timeout = DEFAULT_TIMEOUT;
    tv->tv_usec = *timeout * USEC_PER_MILLISEC;
}

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

    if (getFileSize(filename) < bytesToTransfer)
    {
        bytesToTransfer = getFileSize(filename);
    }

    receive_syn(sockfd, &addr, sizeof(addr));

    unsigned long long bytesSent = 0;
    char packet_data[MAX_BUFFER_SIZE];
    char packet[MAX_BUFFER_SIZE + HEADER_SIZE];
    struct Header header;
    int sequenceNumber = 1;

    struct timeval tv;
    int timeout = DEFAULT_TIMEOUT;
    int retries = 0;
    tv.tv_sec = 0;
    tv.tv_usec = timeout * USEC_PER_MILLISEC;

    while (bytesSent < bytesToTransfer)
    {
        // https://stackoverflow.com/questions/13547721/udp-socket-set-timeout
        tv.tv_usec = timeout * USEC_PER_MILLISEC;

        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
        {
            perror("timeout");
            exit(1);
        }

        // First packet, with no retries yet
        if (retries == 0)
        {
            int bytesRead = fread(packet_data, 1, sizeof(packet_data), file);
            if (bytesRead < 0)
            {
                perror("fread");
                exit(1);
            }

            // Copy in packet data
            memcpy(packet + HEADER_SIZE, packet_data, bytesRead);

            // Add header to packet
            header.sequenceNumber = sequenceNumber;
            header.messageLength = bytesRead;
            memcpy(packet, &header, HEADER_SIZE);
        }

        // https://www.ibm.com/docs/en/zos/3.1.0?topic=functions-sendto-send-data-socket
        int bytesSentThisTime = sendto(sockfd, packet, HEADER_SIZE + MAX_BUFFER_SIZE, 0, (struct sockaddr *)&addr, sizeof(addr));
        if (bytesSentThisTime < 0)
        {
            perror("sendto");
            exit(1);
        }

        checkAck(addr, sockfd, &sequenceNumber, &bytesSent, bytesSentThisTime, &timeout, &retries, &tv);
    }

    // Send null terminating packet
    packet[HEADER_SIZE] = '\0';
    int bytesSentThisTime = sendto(sockfd, packet, HEADER_SIZE + MAX_BUFFER_SIZE, 0, (struct sockaddr *)&addr, sizeof(addr));
    if (bytesSentThisTime < 0)
    {
        perror("sendto");
        exit(1);
    }

    checkAck(addr, sockfd, &sequenceNumber, &bytesSent, bytesSentThisTime, &timeout, &retries, &tv);

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

    rsend(hostname, hostUDPport, filename, bytesToTransfer);

    return (EXIT_SUCCESS);
}