#include <omnetpp.h>
#include "frame_m.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

typedef unsigned int seq_nr;                                        
typedef enum
{
    DATA,
    ACK,
    NAK
} frame_kind;

typedef enum
{
    FRAME_ARRIVAL,
    CHECKSUM_ERROR,
    TIMEOUT,
    NETWORK_LAYER_READY,
    ACK_TIMEOUT
} event_type;

#define MAX_SEQ 1000
#define NR_BUFS ((MAX_SEQ + 1) / 2)
#define inc(k) if(k < MAX_SEQ) k++; else k = 0
bool no_ack = true;
seq_nr old_frame = MAX_SEQ - 1;                                     // seq_nr is not colored green
