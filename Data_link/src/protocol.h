#include <omnetpp.h>
#include "frame_m.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#define MAX_PKT 1024
#define MAX_SEQ 1000
#define NR_BUFS ((MAX_SEQ + 1) / 2)
#define inc(k) if(k < MAX_SEQ) k++; else k = 0

typedef int seq_nr;
typedef int ack_nr;
typedef struct {std::string ptext;} packet;                                        
typedef enum
{
    DATA,
    ACK,
    NACK
} frame_kind;

typedef enum
{
    FRAME_ARRIVAL,
    CHECKSUM_ERROR,
    TIMEOUT,
    NETWORK_LAYER_READY,
    ACK_TIMEOUT
} event_type;

bool no_nack = true;
seq_nr old_frame = MAX_SEQ - 1;
