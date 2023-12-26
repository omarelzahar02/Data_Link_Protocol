#include "Node.h"

#include "./initMsg_m.h"
Define_Module(Node);

void Node::initialize() {
    // TODO - Generated method body
    readfile();
    packet *temp = new packet;
    from_network_layer(temp);

    WS = getParentModule()->par("WS").intValue();
    WR = getParentModule()->par("WR").intValue();
    TO = getParentModule()->par("TO").intValue();
    TD = getParentModule()->par("TD").doubleValue();
    PT = getParentModule()->par("PT").doubleValue();
    ED = getParentModule()->par("ED").doubleValue();
    DD = getParentModule()->par("DD").doubleValue();

    next_frame_to_send = 0;
    ack_expected = 0;
    frame_expected = 0;
    too_far = NR_BUFS;
    nbuffered = 0;
    oldest_frame = 0;
    no_nack = true;
    for (i = 0; i < NR_BUFS; i++) {
        arrived[i] = false;
    }
    /////////////////////////////////////////////////////////////////////////////////////
    int x = 0;
    while (true) {
        // wait_for_event();
        event = NETWORK_LAYER_READY;

        switch (event) {
            case NETWORK_LAYER_READY:
                nbuffered++;
                // from_network_layer(&outbuffer[next_frame_to_send % NR_BUFS]);
                send_frame(DATA, next_frame_to_send, frame_expected, outbuffer);
                inc(next_frame_to_send);
                x++;
                break;
            case FRAME_ARRIVAL:
                // from_physical_layer(r);
                if (r->getM_Type() == DATA) {
                    if ((r->getSeq_Num() != frame_expected) && no_nack)
                        send_frame(NACK, 0, frame_expected, outbuffer);
                    else
                        // start_ack_timer();                                                      //maybe not used
                        if (between(frame_expected, r->getSeq_Num(), too_far) && (arrived[r->getSeq_Num() % NR_BUFS] == false)) {
                            arrived[r->getSeq_Num() % NR_BUFS] = true;
                            inbuffer[r->getSeq_Num() % NR_BUFS] = {r->getM_Payload()};
                            while (arrived[frame_expected % NR_BUFS]) {
                                // to_network_layer(&inbuffer[frame_expected % NR_BUFS]);              //will be replaced by printing
                                no_nack = true;
                                arrived[frame_expected % NR_BUFS] = false;
                                inc(frame_expected);
                                inc(too_far);
                                // start_ack_timer();                                                  //maybe not used
                            }
                        }
                }
                if ((r->getM_Type() == NACK) && between(ack_expected, (r->getAck() + 1) % (MAX_SEQ + 1), next_frame_to_send))
                    send_frame(DATA, (r->getAck() + 1) % (MAX_SEQ + 1), frame_expected, outbuffer);
                while (between(ack_expected, r->getAck(), next_frame_to_send)) {
                    nbuffered--;
                    // stop_timer(ack_expected % NR_BUFS);                                           //bool is needed
                    inc(ack_expected);
                }
                break;
            case CHECKSUM_ERROR:
                if (no_nack)
                    send_frame(NACK, 0, frame_expected, outbuffer);
                break;
            case TIMEOUT:
                send_frame(DATA, ack_expected, frame_expected, outbuffer);
                break;
            case ACK_TIMEOUT:
                send_frame(ACK, 0, frame_expected, outbuffer);
                break;
        }
        // if(nbuffered < NR_BUFS)
        //     enable_network_layer();
        // else
        //     disable_network_layer();
        if (x == 4)
            break;
    }
}
void Node::handleMessage(cMessage *msg) {
    cout << "******************"
         << " Node: " << getName() << " Received Message "
         << "*****************************" << endl;

    try {
        InitMsg_Base *received_msg = check_and_cast<InitMsg_Base *>(msg);
        handleCoordiantionMessage(received_msg);
        return;
    } catch (...) {
        cout << "Not a configuration message" << endl;
    }

    try {
        Frame_Base *received_frame = check_and_cast<Frame_Base *>(msg);
        handleFrameMessage(received_frame);
    } catch (...) {
        cout << "Not a frame" << endl;
    }

    cout << "**************Finished Handling Message************" << endl;
}

void Node::readFile() {
    ifstream file;
    vector<string> lines;
    cout << getName() << " is initialized" << endl;

    if (strcmp(getName(), "Node0") == 0)
        file.open("../src/input0.txt");
    else
        file.open("../src/input1.txt");
    if (!file.is_open()) {
        cerr << "Error opening file: " << strerror(errno) << endl;
    } else {
        string line;
        while (getline(file, line)) {
            lines.push_back(line);
        }
        // std::cout << lines.size() << std::endl;
    }
    file.close();

    for (int i = 0; i < lines.size(); i++) {
        status.push_back(lines[i].substr(0, 4));
        data.push_back(lines[i].substr(5, lines[i].size()));
    }
    for (int i = 0; i < lines.size(); i++) {
        // std::cout << status[i] << std::endl;
        // std::cout << data[i] << std::endl;
    }
}

bool Node::between(seq_nr a, seq_nr b, seq_nr c)  // maybe function will be needed to be static
{
    if (((a <= b) && (b < c)) || ((c < a) && (a <= b)) || ((b < c) && (c < a)))
        return true;
    else
        return false;
}

void Node::from_network_layer(packet *p) {
    p->ptext = data[0];
    data.erase(data.begin());
    // std::cout << "Data sent to physical layer: " << p->ptext << std::endl;
}

void Node::to_physical_layer(Frame_Base *s) {
    // send(s,"out");
}

void Node::start_timer(seq_nr k, bool &timer_on) {
    timer_on = true;
    timerMessage = new cMessage(("timer" + to_string(k)).c_str());
    scheduleAt(simTime() + 1, timerMessage);
}

void Node::stop_timer(seq_nr k, bool &timer_on) {
    timer_on = false;
}

void Node::send_frame(frame_kind fk, seq_nr frame_nr, seq_nr frame_expected, packet buffer[]) {
    Frame_Base s;
    // cout << "ana geet" << endl;
    s.setM_Type(fk);
    if (fk == DATA) {
        // error and delay will be handeled here
        s.setM_Payload(buffer[frame_nr % NR_BUFS].ptext.c_str());
        check_sum(&s);
    }
    s.setSeq_Num(frame_nr);
    s.setAck((frame_expected + MAX_SEQ) % (MAX_SEQ + 1));
    if (fk == NACK) {
        no_nack = false;
    }
    to_physical_layer(&s);  // function needs to be implemented
    if (fk == DATA) {
        bool timer_on = false;                      // for testing
        start_timer(frame_nr % NR_BUFS, timer_on);  // function needs to be implemented
    }
    // stop_ack_timer();                                                         //function needs to be implemented
}

Frame_Base *Node::createFrame(Frame_Base *frame, std::string str, frame_kind fk, seq_nr frame_nr) {
    check_sum(frame, str);
    std::string temp = byte_stuffing(str);
    frame->setM_Payload(temp.c_str());
    frame->setSeq_Num(frame_nr);
    frame->setAck((frame_expected + MAX_SEQ) % (MAX_SEQ + 1));
    frame->setM_Type(fk);

    return frame;
}

void Node::check_sum(Frame_Base *s, std::string payload) {
    std::bitset<8> temp(0);
    // std::string payload = s->getM_Payload();
    for (int i = 0; i < payload.size(); i++) {
        temp = temp ^ bitset<8>(payload[i]);
    }
    s->setMycheckbits(temp);
}

void Node::initNode(node_type type, timing startingTime) {
    setNodeType(type);
    setNodeStartingTime(startingTime);
}

void Node::setNodeStartingTime(timing startingTime) {
    this->startingTime = startingTime;
    cout << "Node starting time is: " << startingTime << endl;
}

void Node::setNodeType(node_type type) {
    this->type = type;
    if (type == SENDER) {
        cout << "Node is a sender" << endl;
    } else {
        cout << "Node is a receiver" << endl;
    }
}

void Node::simulate_sending(int Modification, int Loss, int Duplication, int Delay) {
    int transmission_delay = par("TD").intValue();
    int processing_delay = par("PT").intValue();
    int error_delay = par("ED").intValue();
    int duplication_delay = par("DD").intValue();
    int totalTime = transmission_delay + processing_delay;

    if (Modification == 1) {
        totalTime += error_delay;
    }

    if (Loss == 1) {
        totalTime = processing_delay;
    }
}

std::string Node::byte_stuffing(std::string str) {
    std::string result = "";
    result += FLAG_BYTE;
    for (int i = 0; i < str.size(); i++) {
        if (str[i] == FLAG_BYTE || str[i] == ESCAPE_BYTE) {
            result += ESCAPE_BYTE;
        }
        result += str[i];
    }
    result += FLAG_BYTE;
    return result;
}
