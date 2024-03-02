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

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <pthread.h>
#include <errno.h>
#include "include/udp.h"

/**
 * @brief Sends an acknowledgment message to the sender.
 *
 * TODO: add sequence number to ack_msg (depends on how this is implemented in sender)
 *
 * @param sockfd
 * @param addr
 * @param addrlen
 */
void send_ack(int sockfd, struct sockaddr_in *addr, socklen_t addrlen, uint32_t sequenceNumber)
{
    // https://www.ibm.com/docs/en/zos/3.1.0?topic=functions-sendto-send-data-socket
    if (sendto(sockfd, &sequenceNumber, sizeof(uint32_t), 0, (struct sockaddr *)addr, addrlen) == -1)
    {
        perror("sendto");
        exit(EXIT_FAILURE);
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
        send_ack(sockfd, &addr, addrlen, header.sequenceNumber);

        char packet_data[header.messageLength];
        memcpy(packet_data, packet + HEADER_SIZE, header.messageLength);

        char *nullByte = memchr(packet_data, '\0', header.messageLength);
        if (nullByte != NULL)
        {
            break;
        }

        fwrite(packet_data, 1, header.messageLength, file);

        bytesWritten += bytesReceived;
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
 *  writeRate is an optional parameter that is passed in via the command line.
 *  By default, writeRate is set to 0.
 *
 *  @return Should not return
 */
int main(int argc, char **argv)
{

    unsigned short int udpPort;
    char *destinationFile = NULL;
    unsigned long long int writeRate;

    udpPort = (unsigned short int)atoi(argv[1]);
    destinationFile = argv[2];

    if (argc == 4)
    {
        writeRate = (unsigned long long int)atoll(argv[3]);
    }
    else if (argc == 3)
    {
        writeRate = 0;
    }
    else
    {
        fprintf(stderr, "usage: %s UDP_port filename_to_write [writeRate]\n\n", argv[0]);
        exit(1);
    }

    rrecv(udpPort, destinationFile, writeRate);

    return (EXIT_SUCCESS);
}
