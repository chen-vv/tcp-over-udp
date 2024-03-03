#ifndef UDP_H
#define UDP_H

#include <stdint.h> // For uint32_t

#define FALSE 0
#define TRUE 1

#define HEADER_SIZE sizeof(struct Header)
#define USEC_PER_MILLISEC 1000
#define MAX_BUFFER_SIZE 2
#define DEFAULT_TIMEOUT 100
#define MAX_RETRIES 3
#define MAX_ACK_SIZE sizeof(uint32_t)
#define SYN_ACK_TIMEOUT_MILLISEC 100
#define SYN_ACK_MAX_TIMEOUT 1600

struct Header
{
    uint32_t sequenceNumber;
    uint32_t messageLength;
};

struct Syn
{
    uint32_t sequenceNumber;
};

struct Ack
{
    uint32_t ackNumber;
};

struct SynAck
{
    uint32_t sequenceNumber;
    uint32_t ackNumber;
};

#endif // UDP_H
