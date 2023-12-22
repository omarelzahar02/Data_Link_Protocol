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

#include "Node.h"


Define_Module(Node);

void Node::initialize()
{
    // TODO - Generated method body
    readfile();
    packet *temp = new packet;
    from_network_layer(temp);

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

    //enable_network_layer();
    ack_expected = 0;
    next_frame_to_send = 0;
    frame_expected = 0;
    too_far = NR_BUFS;
    nbuffered = 0;
    for (i = 0; i < NR_BUFS; i++)
    {
        arrived[i] = false;
    }
    /////////////////////////////////////////////////////////////////////////////////////
    int x = 0;
    while(true)
    {
        //wait_for_event();
        event = NETWORK_LAYER_READY;

        switch(event)
        {
            case NETWORK_LAYER_READY:
                nbuffered++;

                //from_network_layer(&outbuffer[next_frame_to_send % NR_BUFS]);
                send_frame(DATA, next_frame_to_send, frame_expected, outbuffer);
                inc(next_frame_to_send);
                x++;
                break;
            case FRAME_ARRIVAL:
                //from_physical_layer(r);
                if(r->getM_Type() == DATA)
                {
                    if ((r->getSeq_Num() != frame_expected) && no_nack)
                        send_frame(NACK, 0, frame_expected, outbuffer);
                    else
                        //start_ack_timer();                                                      //maybe not used
                    if (between(frame_expected, r->getSeq_Num(), too_far) && (arrived[r->getSeq_Num() % NR_BUFS] == false))
                    {
                        arrived[r->getSeq_Num() % NR_BUFS] = true;
                        inbuffer[r->getSeq_Num() % NR_BUFS] = {r->getM_Payload()};
                        while (arrived[frame_expected % NR_BUFS])
                        {
                            //to_network_layer(&inbuffer[frame_expected % NR_BUFS]);              //will be replaced by printing
                            no_nack = true;
                            arrived[frame_expected % NR_BUFS] = false;
                            inc(frame_expected);
                            inc(too_far);
                            //start_ack_timer();                                                  //maybe not used
                        }
                    }
                }
                if ((r->getM_Type() == NACK) && between(ack_expected, (r->getAck() + 1) % (MAX_SEQ + 1), next_frame_to_send))
                    send_frame(DATA, (r->getAck() + 1) % (MAX_SEQ + 1), frame_expected, outbuffer);
                while (between(ack_expected, r->getAck(), next_frame_to_send))
                {
                    nbuffered--;
                    //stop_timer(ack_expected % NR_BUFS);                                           //bool is needed
                    inc(ack_expected);
                }
                break;
        }
        if (x == 4)
            break;
    }
}

void Node::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
    /////////////////////////////////////////////////////////////////////////////////////
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

    //enable_network_layer();
    ack_expected = 0;
    next_frame_to_send = 0;
    frame_expected = 0;
    too_far = NR_BUFS;
    nbuffered = 0;
    for (i = 0; i < NR_BUFS; i++)
    {
        arrived[i] = false;
    }
    /////////////////////////////////////////////////////////////////////////////////////

}

void Node::readfile()
{
    std::ifstream file;
    std::vector<std::string> lines;
    std::cout << getName() << " is initialized" << std::endl;
    
    if(strcmp(getName(),"Node0") == 0)
        file.open("../src/input0.txt");
    else
        file.open("../src/input1.txt");
    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << strerror(errno) << std::endl;
    }
    else
    {
        std::string line;
        while (getline(file, line))
        {
            lines.push_back(line);
        }
        std::cout << lines.size() << std::endl;
    }
    file.close();

    for (int i = 0; i < lines.size(); i++)
    {
        status.push_back(lines[i].substr(0, 4));
        data.push_back(lines[i].substr(5, lines[i].size()));
        // std::vector<unsigned char> vec(lines[i].begin(), lines[i].end());
        // data.push_back(vec);

        //std::cout<<status[i]<<std::endl;
        //std::cout<<data[i]<<std::endl;
    }
    for (int i = 0; i < lines.size(); i++)
    {
        //std::cout << status[i] << std::endl;
        std::cout<<data[i]<<std::endl;
    }
}

bool Node::between(seq_nr a, seq_nr b, seq_nr c)                          // maybe function will be needed to be static
{
    if (((a <= b) && (b < c)) || ((c < a) && (a <= b)) || ((b < c) && (c < a)))
        return true;
    else
        return false;
}

void Node::from_network_layer(packet *p)
{
    p->ptext = data[0];
    data.erase(data.begin());
    std::cout << "Data sent to physical layer: " << p->ptext << std::endl;
}

void Node::to_physical_layer(Frame_Base *s)
{
    //send(s,"out");
}

void Node::start_timer(seq_nr k, bool &timer_on)
{
    timer_on = true;
    timerMessage = new cMessage(("timer" + std::to_string(k)).c_str());
    scheduleAt(simTime() + 1, timerMessage);
}

void Node::stop_timer(seq_nr k, bool &timer_on)
{
    timer_on = false;
}

void Node::send_frame(frame_kind fk, seq_nr frame_nr, seq_nr frame_expected, packet buffer[])
{
    Frame_Base s;
    std::cout << "ana geet" <<std::endl;
    s.setM_Type(fk);
    if(fk == DATA)
    {
        // error and delay will be handeled here
        s.setM_Payload(buffer[frame_nr % NR_BUFS].ptext.c_str());
        check_sum(&s);
    }
    s.setSeq_Num(frame_nr);
    s.setAck((frame_expected + MAX_SEQ) % (MAX_SEQ + 1));
    if(fk == NACK)
    {
        no_nack = false;
    }
    to_physical_layer(&s);                                                      //function needs to be implemented
    if(fk == DATA)
    {
        bool timer_on = false;                                                  // for testing
        start_timer(frame_nr % NR_BUFS, timer_on);                              //function needs to be implemented
    }
    //stop_ack_timer();                                                         //function needs to be implemented
}

void Node::check_sum(Frame_Base *s)
{
    std::bitset<8> temp(0);
    std::string payload = s->getM_Payload();
    for(int i = 0; i < payload.size(); i++)
    {
        temp = temp ^ std::bitset<8>(payload[i]);
    }
    s->setMycheckbits(temp);
}
