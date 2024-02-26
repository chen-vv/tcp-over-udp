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

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <pthread.h>
#include <errno.h>

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

    if (argc != 5) {
        fprintf(stderr, "usage: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n\n", argv[0]);
        exit(1);
    }
    hostUDPport = (unsigned short int) atoi(argv[2]);
    hostname = argv[1];
    bytesToTransfer = atoll(argv[4]);

    return (EXIT_SUCCESS);
}