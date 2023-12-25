//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

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
/**
 * TODO - Generated class
 */
class Node : public cSimpleModule {
   private:
    int WS;                                                     // Sender window size
    int WR;                                                     // Receiver window size
    int TO;                                                     // Timeout
    double PT;                                                  // Processing time
    double TD;                                                  // Transmission delay
    double ED;                                                  // Error delay
    double DD;                                          
    std::vector<std::string> data;
    std::vector<std::string> status;
    node_type type;
    cMessage *timerMessage;
    timing startingTime;
    
    seq_nr ack_expected;
    seq_nr next_frame_to_send;
    seq_nr frame_expected;
    seq_nr too_far;
    seq_nr nbuffered;
    seq_nr i;
    event_type event;
    Frame_Base *r = new Frame_Base;
    packet inbuffer[NR_BUFS];
    packet outbuffer[NR_BUFS];
    bool arrived[NR_BUFS];

    void setNodeType(node_type type);
    void setNodeStartingTime(timing startingTime);
    void initNode(node_type type, timing startingTime = 0);

   protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void readfile();
    virtual bool between(seq_nr a, seq_nr b, seq_nr c);
    virtual void from_network_layer(packet *p);
    // virtual void to_network_layer(packet *p);
    virtual void to_physical_layer(Frame_Base *s);
    // virtual void from_physical_layer(Frame_Base *s);
    virtual void start_timer(seq_nr k, bool &timer_on);
    virtual void stop_timer(seq_nr k, bool &timer_on);
    // virtual void start_ack_timer(void);
    // virtual void stop_ack_timer(void);
    // virtual void enable_network_layer(void);
    // virtual void disable_network_layer(void);
    // virtual void wait_for_event(event_type *event);
    virtual void send_frame(frame_kind fk, seq_nr frame_nr, seq_nr frame_expected, packet buffer[]);
    virtual void check_sum(Frame_Base *s, std::string payload);
    virtual void simulate_sending(int Modification, int Loss, int Duplication, int Delay);
    virtual std::string byte_stuffing(std::string str);
    //virtual std::string byte_destuffing(Frame_Base *s);
    virtual Frame_Base *createFrame(Frame_Base *frame, std::string str);

};

#endif
