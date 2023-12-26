#ifndef __DATA_LINK_NODE_H_
#define __DATA_LINK_NODE_H_

#include <omnetpp.h>

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "frame_m.h"
#include "protocol.h"

using namespace omnetpp;
using namespace std;
class Node : public cSimpleModule {
   private:
    /*Simulation Variables*/
    vector<string> data;    // data to be sent
    vector<string> status;  // simulation status of data: Delay, Duplication, Loss, Modification
    node_type type;         // node type: sender or receiver
    timing startingTime;    // node starting time
    cMessage *timerMessage;

    /*Protocol Variables*/
    seq_nr next_frame_to_send;
    seq_nr ack_expected;
    seq_nr frame_expected;
    seq_nr too_far;
    seq_nr nbuffered;
    packet inbuffer[NR_BUFS];
    packet outbuffer[NR_BUFS];
    bool arrived[NR_BUFS];
    bool no_nack;
    seq_nr old_frame;

    /*Simulation Functions*/
    void setNodeType(node_type type);
    void handleCoordiantionMessage(InitMsg_Base *msg);
    void handleFrameMessage(Frame_Base *msg);
    void setNodeStartingTime(timing startingTime);
    void initNode(node_type type, timing startingTime = 0);
    void readFile();

    /*Protocol Functions*/
    bool between(seq_nr a, seq_nr b, seq_nr c);
    void from_network_layer(packet *p);
    // void to_network_layer(packet *p);
    void to_physical_layer(Frame_Base *s);
    // void from_physical_layer(Frame_Base *s);
    void start_timer(seq_nr k, bool &timer_on);
    void stop_timer(seq_nr k, bool &timer_on);
    // void start_ack_timer(void);
    // void stop_ack_timer(void);
    // void enable_network_layer(void);
    // void disable_network_layer(void);
    // void wait_for_event(event_type *event);
    void send_frame(frame_kind fk, seq_nr frame_nr, seq_nr frame_expected, packet buffer[]);
    void check_sum(Frame_Base *s);
    void simulate_sending(int Modification, int Loss, int Duplication, int Delay);

   protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

#endif
