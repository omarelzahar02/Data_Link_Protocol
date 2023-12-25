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
#define NR_BUFS ((MAX_SEQ + 1) / 2)
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
typedef struct {
    std::string ptext;
} packet;
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
static bool no_nack = true;
static seq_nr old_frame = MAX_SEQ - 1;

#endif
