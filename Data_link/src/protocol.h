#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <omnetpp.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "frame_m.h"

#define MAX_PKT 1024
#define MAX_SEQ 1000
#define RECHECK_FOR_MSGS_TIME 0.01
#define IS_NETWORK_LAYER_READY 0
#define TIMER_TIME_OUT 1

#define inc(k)       \
    if (k < MAX_SEQ) \
        k++;         \
    else             \
        k = 0

#define FLAG_BYTE '#'
#define ESCAPE_BYTE '/'

typedef int seq_nr;
typedef int ack_nr;
typedef int timing;

typedef std::string Packet;

typedef enum {
    DATA,
    ACK,
    NACK
} frame_kind;

typedef enum {
    FRAME_ARRIVAL,
    CHECKSUM_ERROR,
    TIMEOUT,
    NETWORK_LAYER_READY,
    ACK_TIMEOUT
} event_type;

typedef enum {
    SENDER,
    RECEIVER
} node_type;

#endif
