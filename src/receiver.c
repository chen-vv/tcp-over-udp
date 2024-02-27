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

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <pthread.h>
#include <errno.h>

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
            char* destinationFile, 
            unsigned long long int writeRate)
{
    // https://www.geeksforgeeks.org/socket-programming-cc/
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    // https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html
    struct sockaddr_in *addr = NULL;
    addr->sin_family = AF_INET;
    addr->sin_port = htons(myUDPport);
    addr->sin_addr.s_addr = htonl(INADDR_ANY);

    socklen_t addrlen = sizeof(addr);

    int err = bind(sockfd, (struct sockaddr*) addr, addrlen);
    if (err < 0) {
        perror("bind");
        exit(1);
    }

    FILE *file = fopen(destinationFile, "wb");
    if (file == NULL) {
        perror("fopen");
        exit(1);
    }
}


/** @brief UDP receiver entrypoint.
 *
 *  TODO: Implement main & write comments
 *
 *  @return Should not return
 */
int main(int argc, char** argv) {
    // This is a skeleton of a main function.
    // You should implement this function more completely
    // so that one can invoke the file transfer from the
    // command line.

    unsigned short int udpPort;

    if (argc != 3) {
        fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
        exit(1);
    }

    udpPort = (unsigned short int) atoi(argv[1]);
}
