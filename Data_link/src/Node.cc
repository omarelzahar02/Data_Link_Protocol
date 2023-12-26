#include "Node.h"

Define_Module(Node);

void Node::initialize() {
    readFile();

    WS = getParentModule()->par("WS").intValue();
    WR = getParentModule()->par("WR").intValue();
    TO = getParentModule()->par("TO").intValue();
    TD = getParentModule()->par("TD").doubleValue();
    PT = getParentModule()->par("PT").doubleValue();
    ED = getParentModule()->par("ED").doubleValue();
    DD = getParentModule()->par("DD").doubleValue();

    inbuffer = vector<Packet>(WS);
    outbuffer = vector<Packet>(WS);
    arrived = vector<bool>(WS);

    next_frame_to_send = 0;
    ack_expected = 0;
    frame_expected = 0;
    NR_BUFS = WS;
    too_far = NR_BUFS;
    nbuffered = 0;
    oldest_frame = 0;
    is_network_layer_enabled = true;
    no_nack = true;
    for (int i = 0; i < NR_BUFS; i++) {
        arrived[i] = false;
    }
}

void Node::handleCoordiantionMessage(InitMsg_Base *receivedMsg) {
    cout << "Received Coordination Message" << endl;
    initNode(receivedMsg->getType(), receivedMsg->getStartTime());
}

void Node::to_network_layer(Packet *p) {
    cout << "Network Layer Received Packet: " << p << endl;
}
void Node::handleReceivingData(Frame_Base *received_msg) {
    /**
     * Handling checksum error
     */
    if (!is_received_data_correct(received_msg)) {
        if (no_nack) {
            send_frame(NACK, 0, frame_expected, outbuffer);
        }
        return;
    }

    if ((received_msg->getSeq_Num() != frame_expected) && no_nack)
        send_frame(NACK, 0, frame_expected, outbuffer);

    if (between(frame_expected, received_msg->getSeq_Num(), too_far) && (arrived[received_msg->getSeq_Num() % NR_BUFS] == false)) {
        arrived[received_msg->getSeq_Num() % NR_BUFS] = true;
        inbuffer[received_msg->getSeq_Num() % NR_BUFS] = received_msg->getM_Payload();
        bool isExpectedReceived = false;
        while (arrived[frame_expected % NR_BUFS]) {
            to_network_layer(&inbuffer[frame_expected % NR_BUFS]);
            no_nack = true;
            arrived[frame_expected % NR_BUFS] = false;
            inc(frame_expected);
            inc(too_far);
            isExpectedReceived = true;
        }
        if (isExpectedReceived) {
            send_frame(ACK, 0, frame_expected, outbuffer);
        }
    }
}

void Node::handleReceivingAck(Frame_Base *received_msg) {
    int lstReceived = (received_msg->getAck() + MAX_SEQ) % (MAX_SEQ + 1);
    while (between(ack_expected, lstReceived, next_frame_to_send)) {
        nbuffered--;
        stop_timer(ack_expected);
        inc(ack_expected);
    }
}

void Node::handleReceivingNack(Frame_Base *received_msg) {
    if ((received_msg->getM_Type() == NACK) && between(ack_expected, received_msg->getAck(), next_frame_to_send))
        send_frame(DATA, received_msg->getAck(), -1, outbuffer);
}

void Node::handleFrameMessage(Frame_Base *received_msg) {
    cout << "Received Frame Message" << endl;
    int frameType = received_msg->getM_Type();
    switch (frameType) {
        case DATA:
            handleReceivingData(received_msg);
            break;
        case NACK:
            handleReceivingNack(received_msg);  // place break here if required
        case ACK:
            handleReceivingAck(received_msg);
            break;
    }
    if (nbuffered < NR_BUFS)
        enable_network_layer();
    else
        disable_network_layer();
}

void Node::terminate() {
    // To call finish function here
    return;
}
void Node::check_for_network_layer_event(void) {
    if (!is_network_layer_enabled) {
        return;
    }
    string simulationParameters;

    bool isThereData = from_network_layer(outbuffer[next_frame_to_send % NR_BUFS], simulationParameters);
    if (!isThereData) {
        terminate();
        return;
    }

    // LOGIC
    send_frame(DATA, next_frame_to_send, frame_expected, outbuffer, simulationParameters);
    inc(next_frame_to_send);
}

void Node::enable_network_layer(void) {
    is_network_layer_enabled = true;
    check_for_network_layer_event();
}

void Node::disable_network_layer(void) {
    is_network_layer_enabled = false;
}

void Node::sendSelfMsg(double sleepDuration, int msgType, int seqNumber) {
    SelfMsg_Base *msg = new SelfMsg_Base("");
    msg->setSelfMsgType(msgType);
    msg->setSeqNumber(seqNumber);
    scheduleAt(simTime() + sleepDuration, msg);
}

void Node::handleTimerTimeOut(int seqNumber) {
    send_frame(DATA, seqNumber, frame_expected, outbuffer);
}

void Node::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        SelfMsg_Base *received_msg = check_and_cast<SelfMsg_Base *>(msg);
        int type = received_msg->getSelfMsgType();
        if (type == IS_NETWORK_LAYER_READY) {
            check_for_network_layer_event();
            sendSelfMsg(RECHECK_FOR_MSGS_TIME, IS_NETWORK_LAYER_READY);
        } else if (type == TIMER_TIME_OUT) {
            handleTimerTimeOut(received_msg->getSeqNumber());
        }
        cancelAndDelete(msg);
        return;
    }

    cout << "******************"
         << " Node: " << getName() << " Received Message "
         << "******************" << endl;

    bool isInitMsg = false;
    try {
        InitMsg_Base *received_msg = check_and_cast<InitMsg_Base *>(msg);
        handleCoordiantionMessage(received_msg);
        if (type == SENDER) {
            sendSelfMsg(RECHECK_FOR_MSGS_TIME, IS_NETWORK_LAYER_READY);
        }
        isInitMsg = true;
    } catch (...) {
        cout << "Not a configuration message" << endl;
    }

    if (!isInitMsg) {
        try {
            Frame_Base *received_frame = check_and_cast<Frame_Base *>(msg);
            handleFrameMessage(received_frame);
        } catch (...) {
            cout << "Not a frame" << endl;
            return;
        }
    }

    cancelAndDelete(msg);

    cout << "**************Finished Handling Message************" << endl;
}

void Node::readFile() {
    ifstream file;
    vector<string> lines;
    cout << getName() << " file is read" << endl;

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
    }
    file.close();

    for (int i = 0; i < lines.size(); i++) {
        status.push(lines[i].substr(0, 4));
        data.push(lines[i].substr(5, lines[i].size()));
    }
}

// maybe function will be needed to be static
bool Node::between(seq_nr a, seq_nr b, seq_nr c) {
    if (((a <= b) && (b < c)) || ((c < a) && (a <= b)) || ((b < c) && (c < a)))
        return true;
    else
        return false;
}

bool Node::from_network_layer(Packet &p, string &simulationParams) {
    if (data.empty())
        return false;
    simulationParams = status.front();
    p = data.front();
    data.pop();
    status.pop();
    return true;
}

void Node::to_physical_layer(Frame_Base *frame, string simulationParams) {
    // send(s,"out");
    // sendDelayed(cMessage *msg, double delay, const char *gateName, int index);
    // sendDelayed(cMessage *msg, double delay, int gateId);
    // sendDelayed(cMessage *msg, double delay, cGate *gate);
    // The arguments are the same as for send(), except for the extra delay parameter. The delay value must be non-negative. The effect of the function is similar to as if the module had kept the message for the delay interval and sent it afterwards; even the sending time timestamp of the message will be set to the current simulation time plus delay.

    // A example call:

    // sendDelayed(msg, 0.005, "out");
    send(frame, "outN");
}

void Node::start_timer(seq_nr seqNum) {
    if (timers.count(seqNum) == 1) {
        stop_timer(seqNum);
    }
    cout << "Start timer on sequence number: " << seqNum << endl;
    SelfMsg_Base *msg = new SelfMsg_Base("");
    msg->setSelfMsgType(TIMER_TIME_OUT);
    msg->setSeqNumber(seqNum);
    timers[seqNum] = msg;
    scheduleAt(simTime() + TO, timers[seqNum]);
}

void Node::stop_timer(seq_nr seqNum) {
    cout << "Stop timer on sequence number: " << seqNum << endl;
    cancelAndDelete(timers[seqNum]);
    timers.erase(seqNum);
}

void Node::send_frame(frame_kind fk, seq_nr frame_nr, seq_nr frame_expected, vector<Packet> &buffer, string simulationParams) {
    Frame_Base *frame;
    if (fk == DATA) {
        frame = create_frame(buffer[frame_nr % NR_BUFS], frame_nr);
    } else {
        frame = new Frame_Base("");
        frame->setM_Type(fk);
        frame->setAck(frame_expected);
        if (fk == NACK) {
            no_nack = false;
        }
    }

    to_physical_layer(frame, simulationParams);
}

Frame_Base *Node::create_frame(string payload, seq_nr frame_nr) {
    Frame_Base *frame = new Frame_Base("");

    bitset<8> checkSum = check_sum(payload);
    frame->setMycheckbits(checkSum);

    string byteStuffedPayload = byte_stuffing(payload);
    frame->setM_Payload(byteStuffedPayload.c_str());
    frame->setSeq_Num(frame_nr);
    // frame->setAck((frame_expected + MAX_SEQ) % (MAX_SEQ + 1));
    frame->setM_Type(DATA);

    return frame;
}

bitset<8> Node::check_sum(string payload) {
    bitset<8> checkSumCalculation(0);
    for (int i = 0; i < payload.size(); i++) {
        checkSumCalculation = checkSumCalculation ^ bitset<8>(payload[i]);
    }
    return checkSumCalculation;
}

bool Node::is_received_data_correct(Frame_Base *s) {
    bitset<8> temp(0);
    string payload = s->getM_Payload();
    for (int i = 0; i < payload.size(); i++) {
        temp = temp ^ bitset<8>(payload[i]);
    }
    return temp == s->getMycheckbits();
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

string Node::byte_stuffing(string str) {
    string result = "";
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
