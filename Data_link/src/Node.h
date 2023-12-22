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
#include "frame_m.h"
#include "protocol.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cerrno>

using namespace omnetpp;

/**
 * TODO - Generated class
 */
class Node : public cSimpleModule
{
  private:
    // std::vector<std::vector<unsigned char>> data;
    std::vector<std::string> data;
    std::vector<std::string> status;
    cMessage *timerMessage;
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
    //virtual void wait_for_event(event_type *event);
    virtual void send_frame(frame_kind fk, seq_nr frame_nr, seq_nr frame_expected, packet buffer[]);
    virtual void check_sum(Frame_Base *s);

};

#endif
