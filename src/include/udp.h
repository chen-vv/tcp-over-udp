#ifndef UDP_H
#define UDP_H

#include <stdint.h> // For uint32_t

#define FALSE 0
#define TRUE 1

#define HEADER_SIZE sizeof(struct Header)
#define USEC_PER_MILLISEC 1000
#define MAX_BUFFER_SIZE 1024
#define DEFAULT_TIMEOUT 100
#define MAX_RETRIES 3
#define MAX_ACK_SIZE sizeof(uint32_t)

struct Header
{
    uint32_t sequenceNumber;
    uint32_t messageLength;
};

#endif // UDP_H
