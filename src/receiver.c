/** @file receiver.c
 *  @brief A UDP receiver program
 *
 *  This contains the code for the receiver
 *  using the UDP file transfer protocol. The
 *  main function parses the command line arguments
 *  and then calls the rrecv function to receive
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
#include <time.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <pthread.h>
#include <errno.h>
#include "include/udp.h"

// Global variable to store the most recent sequence number
uint32_t latestSequenceNumber = 0; // Initialize to an invalid value

/**
 * @brief Sends an acknowledgment message to the sender.
 *
 * TODO: add sequence number to ack_msg (depends on how this is implemented in sender)
 * btw this is for sending an acknowledgement that a packet was received
 *
 * @param sockfd
 * @param addr
 * @param addrlen
 */
void send_packet_ack(int sockfd, struct sockaddr_in *addr, socklen_t addrlen, uint32_t sequenceNumber)
{
    // https://www.ibm.com/docs/en/zos/3.1.0?topic=functions-sendto-send-data-socket
    if (sendto(sockfd, &sequenceNumber, sizeof(uint32_t), 0, (struct sockaddr *)addr, addrlen) == -1)
    {
        perror("sendto");
        exit(EXIT_FAILURE);
    }
}

void receive_syn(int sockfd, struct sockaddr_in *addr, socklen_t addrlen)
{
    printf("Waiting for SYN packet\n");

    // // set socket to non-blocking mode
    // int flags = fcntl(sockfd, F_GETFL, 0);
    // flags |= O_NONBLOCK;
    // if (fcntl(sockfd, F_SETFL, flags) == -1)
    // {
    //     perror("fcntl - F_SETFL");
    //     exit(1);
    // }

    struct timeval tv;
    int timeout = SYN_ACK_TIMEOUT_MILLISEC;
    tv.tv_sec = 0;

    while (1)
    {
        tv.tv_usec = timeout * USEC_PER_MILLISEC;

        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
        {
            perror("timeout");
            exit(1);
        }

        struct Syn syn;

        printf("Listening for bytes\n");
        ssize_t bytes_received = recvfrom(sockfd, &syn, sizeof(struct Syn), 0, (struct sockaddr *)addr, &addrlen);
        printf("Received %zd bytes\n", bytes_received);

        if (bytes_received > 0)
        {
            printf("Received SYN packet with seq: %d\n", syn.sequenceNumber);

            // Reply with syn-ack
            struct SynAck syn_ack;
            syn_ack.sequenceNumber = 777;
            syn_ack.ackNumber = syn.sequenceNumber + 1;

            while (TRUE)
            {
                sendto(sockfd, &syn_ack, sizeof(struct SynAck), 0, (struct sockaddr *)addr, addrlen);

                printf("Sending synack with %d and %d\n", syn_ack.sequenceNumber, syn_ack.ackNumber);
                // Wait for ack
                struct Ack ack;
                ssize_t recv_size = recvfrom(sockfd, &ack, sizeof(struct Ack), 0, (struct sockaddr *)addr, &addrlen);
                if (recv_size > 0)
                {
                    printf("Received ACK packet with ack num: %d\n", ack.ackNumber);
                    return;
                }
                else if (recv_size == -1)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        // ack not received yet
                        printf("ack timeout\n");
                        if (timeout * 2 < SYN_ACK_MAX_TIMEOUT)
                        {
                            timeout *= 2;
                        }
                        continue;
                    }
                    else
                    {
                        perror("recvfrom");
                        exit(1);
                    }
                }
            }
        }
        else if (bytes_received == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // no syn yet
                printf("Syn timeout, new timeout: %d\n", timeout * 2);

                if (timeout * 2 < SYN_ACK_MAX_TIMEOUT)
                {
                    timeout *= 2;
                }

                continue;
            }
            else
            {
                perror("recvfrom");
                exit(1);
            }
        }
    }

    // // revert socket to blocking mode
    // flags &= ~O_NONBLOCK;
    // if (fcntl(sockfd, F_SETFL, flags) == -1)
    // {
    //     perror("fcntl - F_SETFL");
    //     exit(EXIT_FAILURE);
    // }

    // send syn-ack
    char syn = 0;
    ssize_t size = sendto(sockfd, &syn, sizeof(char), 0, (struct sockaddr *)addr, addrlen);
    if (size == -1)
    {
        perror("sendto");
        exit(1);
    }
}

/** @brief Writes the bytes received on port myUDPport to a file
 *         called destinationFile at a rate of writeRate bytes
 *         per second.
 *
 *  If writeRate is 0 then the receiver can write as many bytes
 *  as possible into destinationFile. Otherwise, if writeRate is
 *  non-zero then the receiver should write no more than
 *  writeRate bytes per second to destinationFile. See rsend for
 *  the counterpart function.
 *
 *  @param myUDPport The port number to listen on.
 *  @param destinationFile The name of the file to write to.
 *  @param writeRate The maximum number of bytes to write per second.
 *                   Must be an integer greater than or equal to zero.
 *  @return Void.
 */
void rrecv(unsigned short int myUDPport,
           char *destinationFile,
           unsigned long long int writeRate)
{
    // https://www.geeksforgeeks.org/socket-programming-cc/
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    // https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(myUDPport);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    socklen_t addrlen = sizeof(addr);

    int err = bind(sockfd, (struct sockaddr *)&addr, addrlen);
    if (err < 0)
    {
        perror("bind");
        exit(1);
    }

    FILE *file = fopen(destinationFile, "wb");
    if (file == NULL)
    {
        perror("fopen");
        exit(1);
    }

    // Establish connection with sender
    receive_syn(sockfd, &addr, addrlen);

    // https://www.geeksforgeeks.org/time-function-in-c/
    time_t start, end;
    time(&start);

    unsigned long long bytesWritten = 0;

    while (1)
    {
        // https://www.ibm.com/docs/en/zos/3.1.0?topic=functions-recvfrom-receive-messages-socket
        int packetSize = MAX_BUFFER_SIZE + HEADER_SIZE;
        char packet[packetSize];

        int bytesReceived = recvfrom(sockfd, packet, packetSize, 0, (struct sockaddr *)&addr, &addrlen);
        if (bytesReceived < 0)
        {
            perror("recvfrom");
            exit(1);
        }
        else if (bytesReceived == 0)
        {
            continue;
        }

        struct Header header;
        memcpy(&header, packet, HEADER_SIZE);
        send_packet_ack(sockfd, &addr, addrlen, header.sequenceNumber);

        char packet_data[header.messageLength];
        memcpy(packet_data, packet + HEADER_SIZE, header.messageLength);

        char *nullByte = memchr(packet_data, '\0', header.messageLength);
        if (nullByte != NULL)
        {
            break;
        }

        printf("Executing write...");
        // TODO: If a packet is received with a sequence number that has already been received, discard.
        if (header.sequenceNumber <= latestSequenceNumber)
        {
            printf("Discarding packet with sequence number %d. latest: %d\n", header.sequenceNumber, latestSequenceNumber);
            continue;
        }

        fwrite(packet_data, 1, header.messageLength, file);

        bytesWritten += bytesReceived;
        latestSequenceNumber = header.sequenceNumber;

        time(&end);
        double seconds = difftime(end, start);

        // TODO: implement flow control via sliding window mechanism
        //
        // If writeRate exceeded, signal to sender to slow down
        if (writeRate > 0 && bytesWritten / seconds > writeRate)
        {
            sleep(1);
        }
    }

    fclose(file);
    close(sockfd);
}

/** @brief UDP receiver entrypoint.
 *
 *  TODO: Implement main & write comments
 *
 *  @return Should not return
 */
int main(int argc, char **argv)
{

    unsigned short int udpPort;
    char *destinationFile = NULL;
    unsigned long long int writeRate;

    if (argc != 4)
    {
        fprintf(stderr, "usage: %s UDP_port filename_to_write writeRate\n\n", argv[0]);
        exit(1);
    }

    udpPort = (unsigned short int)atoi(argv[1]);
    destinationFile = argv[2];
    writeRate = (unsigned long long int)atoll(argv[3]);

    rrecv(udpPort, destinationFile, writeRate);

    return (EXIT_SUCCESS);
}
