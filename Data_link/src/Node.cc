#include "Node.h"

Define_Module(Node);

void Node::initialize() {
    clearFile(OUTPUT_FILE);
    readFile();

    WS = getParentModule()->par("WS").intValue();
    WR = getParentModule()->par("WR").intValue();
    TO = getParentModule()->par("TO").doubleValue();
    TD = getParentModule()->par("TD").doubleValue();
    PT = getParentModule()->par("PT").doubleValue();
    ED = getParentModule()->par("ED").doubleValue();
    DD = getParentModule()->par("DD").doubleValue();
    wakeUpMsg = NULL;

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
    EV << "Network Layer Received Frame: " << *p << endl;
}
void Node::handleReceivingData(Frame_Base *received_msg) {
    /**
     * Handling checksum error
     */
    string receivedPayload = received_msg->getM_Payload();
    string deStuffedPayload = byte_destuffing(receivedPayload);
    received_msg->setM_Payload(deStuffedPayload.c_str());

    if (!is_received_data_correct(received_msg)) {
        writeRecievedToOutputFile(received_msg, true, 0, receivedPayload);
        EV << "CheckSum Error" << endl;
        if (no_nack) {
            send_frame(NACK, 0, frame_expected, outbuffer);
        }
        return;
    }

    if (between(frame_expected, received_msg->getSeq_Num(), too_far) && (arrived[received_msg->getSeq_Num() % NR_BUFS] == true)) {
        writeRecievedToOutputFile(received_msg, false, 1, receivedPayload);
        EV << "Received duplicated frame with sequence number " << received_msg->getSeq_Num() << endl;
    }
    writeRecievedToOutputFile(received_msg, false, 0, receivedPayload);

    if ((received_msg->getSeq_Num() != frame_expected) && no_nack) {
        send_frame(NACK, 0, frame_expected, outbuffer);
    }

    if (between(frame_expected, received_msg->getSeq_Num(), too_far) && (arrived[received_msg->getSeq_Num() % NR_BUFS] == false)) {
        EV << "Frame Expected " << frame_expected << " Curr Frame " << received_msg->getSeq_Num()
           << " Too far " << too_far << endl;
        arrived[received_msg->getSeq_Num() % NR_BUFS] = true;
        inbuffer[received_msg->getSeq_Num() % NR_BUFS] = received_msg->getM_Payload();
        bool isExpectedReceived = false;
        while (arrived[frame_expected % NR_BUFS]) {
            EV << "Cumulative Ack on sequence Number " << frame_expected << endl;
            to_network_layer(&inbuffer[frame_expected % NR_BUFS]);
            no_nack = true;
            arrived[frame_expected % NR_BUFS] = false;
            inc(frame_expected);
            inc(too_far);
            isExpectedReceived = true;
        }
        if (isExpectedReceived) {
            EV << "Send Cumulative Ack" << endl;
            send_frame(ACK, 0, frame_expected, outbuffer);
        } else {
            EV << "Received frame within window with sequence number " << received_msg->getSeq_Num() << endl;
        }
    }
}

void Node::handleReceivingAck(Frame_Base *received_msg) {
    int lstReceived = (received_msg->getAck() + MAX_SEQ) % (MAX_SEQ + 1);
    while (between(ack_expected, lstReceived, next_frame_to_send)) {
        EV << "Sender Acks on frame " << ack_expected << endl;
        nbuffered--;
        stop_timer(ack_expected);
        inc(ack_expected);
    }
}

void Node::handleReceivingNack(Frame_Base *received_msg) {
    if ((received_msg->getM_Type() == NACK) && between(ack_expected, received_msg->getAck(), next_frame_to_send)) {
        EV << "At time " << simTime() << " Received NACK on sequence number " << received_msg->getAck() << endl;
        send_frame(DATA, received_msg->getAck(), -1, outbuffer, "0000", 1);
    }
}

void Node::handleFrameMessage(Frame_Base *received_msg) {
    EV << "At time " << simTime() << " Received Frame Message" << endl;
    EV << "Windows' pointers before processing incoming frame" << endl;

    if (type == SENDER) {
        EV << "Ack expected " << ack_expected << " Next frame to send " << next_frame_to_send << endl;
    } else {
        EV << "Frame expected " << frame_expected << " Too far " << too_far << endl;
    }

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
    EV << "Windows' pointers after processing incoming frame" << endl;
    if (type == SENDER) {
        EV << "Ack expected " << ack_expected << " Next frame to send " << next_frame_to_send << endl;
    } else {
        EV << "Frame expected " << frame_expected << " Too far " << too_far << endl;
    }

    if (nbuffered < NR_BUFS)
        enable_network_layer();
    else
        disable_network_layer();
}

void Node::terminate() {
    // To call finish function here
    if (timers.size() == 0) {
        EV << "Ending Simulation" << endl;
        finish();
        endSimulation();
    }
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
    nbuffered++;
    inc(next_frame_to_send);

    if (nbuffered < NR_BUFS)
        enable_network_layer();
    else
        disable_network_layer();
}

void Node::enable_network_layer(void) {
    is_network_layer_enabled = true;
    if (type == SENDER && simTime() >= lstProcessingFinish) {
        setNextWakeUpAfterTime(0);
    }
}

void Node::disable_network_layer(void) {
    is_network_layer_enabled = false;
}

void Node::setNextWakeUpAfterTime(double time) {
    if (wakeUpMsg == NULL) return;
    cancelAndDelete(wakeUpMsg);
    sendSelfMsg(time, IS_NETWORK_LAYER_READY);
}

void Node::sendSelfMsg(double sleepDuration, int msgType, int seqNumber) {
    SelfMsg_Base *msg = new SelfMsg_Base("");
    msg->setSelfMsgType(msgType);
    msg->setSeqNumber(seqNumber);
    scheduleAt(simTime() + sleepDuration, msg);
    wakeUpMsg = msg;
}

void Node::handleTimerTimeOut(int seqNumber) {
    EV << "Handling time out at sender for sequence number " << seqNumber << endl;
    send_frame(DATA, seqNumber, frame_expected, outbuffer, "0000", 1);
    writeToTimeOutFile(seqNumber);
}

bool Node::checkForProcessingFinishToDisplay(cMessage *msg) {
    string response = msg->getName();
    const string scheduledIdentifier = "scheduled";
    int scheduledIdentifierSize = scheduledIdentifier.size();
    if (response.size() >= scheduledIdentifierSize && response.substr(0, scheduledIdentifierSize) == scheduledIdentifier) {
        string printToFileProcessingFinished = response.substr(9);
        writeStringToFile(printToFileProcessingFinished);
        cancelAndDelete(msg);
        return 1;
    }
    return 0;
}

bool Node::checkForDelayedMsgsToSend(cMessage *msg) {
    try {
        Frame_Base *frameToSend = check_and_cast<Frame_Base *>(msg);
        send(frameToSend, "outN");
        return 1;
    } catch (...) {
        return 0;
    }
}

void Node::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        if (checkForProcessingFinishToDisplay(msg)) {
            return;
        } else if (checkForDelayedMsgsToSend(msg)) {
            return;
        }
        SelfMsg_Base *received_msg = check_and_cast<SelfMsg_Base *>(msg);
        int mType = received_msg->getSelfMsgType();
        if (mType == IS_NETWORK_LAYER_READY) {
            check_for_network_layer_event();
        } else if (mType == TIMER_TIME_OUT) {
            handleTimerTimeOut(received_msg->getSeqNumber());
        }
        // cancelAndDelete(msg);
        return;
    }

    cout << "******************"
         << " Node: " << getName() << " Received Message "
         << "******************" << endl;

    bool isInitMsg = false;
    if (isFirstTime) {
        isFirstTime = false;
        try {
            InitMsg_Base *received_msg = check_and_cast<InitMsg_Base *>(msg);
            handleCoordiantionMessage(received_msg);
            if (type == SENDER) {
                sendSelfMsg(startingTime, IS_NETWORK_LAYER_READY);
            }
            isInitMsg = true;
        } catch (...) {
            cout << "Not a configuration message" << endl;
        }
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
        file.open(INPUT_FILE_0);
    else
        file.open(INPUT_FILE_1);
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

string Node::modifyRandomBit(string s, int beg, int end, int &randPos) {
    int size = end - beg + 1;
    randPos = beg + uniform(0, 1) * (double)size;
    int addValue = uniform(1, 25);
    s[randPos] = s[randPos] + addValue;
    return s;
}

void Node::scheduleProcessingMessage(string processingMsg, double processingMsgDelay) {
    if (processingMsg == "")
        return;
    const string uniqueIdentifier = "scheduled";
    string scheduledMsg = uniqueIdentifier + processingMsg;
    cMessage *msg = new cMessage(scheduledMsg.c_str());
    scheduleAt(simTime() + processingMsgDelay, msg);
}

void Node::sendAfter(Frame_Base *frame, double delay, double processingMsgDelay, string processingMsg) {
    scheduleProcessingMessage(processingMsg, processingMsgDelay);
    scheduleAt(simTime() + delay, frame);
}

void Node::to_physical_layer(Frame_Base *frame, string simulationParams, bool isAgain) {
    EV << "********* To Physical Layer *********" << endl;
    EV << "Intended Data for " << getName()
       << "\nType: " << (frame->getM_Type() == 0 ? "DATA" : (frame->getM_Type() == 1 ? "ACK" : "NACK"))
       << "\nPayload: " << frame->getM_Payload() << "\nSeqNum: "
       << frame->getSeq_Num() << "\nAck: " << frame->getAck()
       << "\nChecksum: " << frame->getMycheckbits() << endl;

    double processingTime = PT;
    double transmissionDelay = TD;
    double simulationTime = PT + TD;
    if (frame->getM_Type() != DATA) {
        string finishProcessingMsg = "";
        if (frame->getM_Type() == ACK) {
            finishProcessingMsg = getAckToOutputFileMsg(frame);
        } else {
            finishProcessingMsg = getNackToOutputFileMsg(frame, simulationParams[1] - '0');
        }
        sendAfter(frame, simulationTime, PT, finishProcessingMsg);
        EV << "Current frame is scheduled to be send after " << simulationTime << " at " << simTime() + simulationTime << endl;
        EV << "***************************" << endl;
        return;
    }

    simtime_t currSimTime = simTime();
    simtime_t currProcessingFinish = currSimTime + PT;
    if (currProcessingFinish - lstProcessingFinish < PT) {
        currProcessingFinish = lstProcessingFinish + PT;
    }

    lstProcessingFinish = currProcessingFinish;

    simtime_t processingAndWaitTime = currProcessingFinish - currSimTime;

    EV << "Current node will finish processing at " << simTime() + processingAndWaitTime << endl;
    if (!isAgain)
        writeStartingToOutputFile(frame, simulationParams);
    simulationTime = transmissionDelay + processingAndWaitTime.dbl();

    bool isModification = simulationParams[0] - '0';
    bool isLoss = simulationParams[1] - '0';
    bool isDuplication = simulationParams[2] - '0';
    bool isDelay = simulationParams[3] - '0';
    int modified = -1;

    if (isModification) {
        string before = frame->getM_Payload();
        string after = modifyRandomBit(before, 0, before.size() - 1, modified);
        frame->setM_Payload(after.c_str());
        EV << "Payload is modified\nBefore: " << before
           << "\nAfter: " << frame->getM_Payload() << endl;
    }

    string byteStuffedPayload = byte_stuffing(frame->getM_Payload());
    frame->setM_Payload(byteStuffedPayload.c_str());
    EV << "Frame after byte stuffing: " << byteStuffedPayload << endl;

    setNextWakeUpAfterTime(processingAndWaitTime.dbl());

    if (isLoss) {
        EV << "Current frame will be lost" << endl;
        if (isDuplication) {
            EV << "Current Duplicated frame will be lost" << endl;
            // Printing here
        }
        EV << "***************************" << endl;
        return;
    }

    if (isDelay) {
        EV << "Current frame will be delayed" << endl;
        simulationTime += ED;
    }

    EV << "Current frame is scheduled to be received after " << simulationTime << " at " << simTime() + simulationTime << endl;

    string finishProcessingMsg = getTransmissionToOutputFileMsg(frame, modified, isLoss, isDuplication, isDelay);
    sendAfter(frame, simulationTime, PT, finishProcessingMsg);
    setNextWakeUpAfterTime(processingAndWaitTime.dbl());
    if (isDuplication) {
        EV << "Current frame will be duplicated" << endl;
        simulationTime += DD;
        EV << "Duplicated frame is scheduled to be received after " << simulationTime << " at " << simTime() + simulationTime << endl;
        string finishProcessingMsg = getTransmissionToOutputFileMsg(frame, modified, isLoss, 2, isDelay);
        sendAfter(new Frame_Base(*frame), simulationTime, PT + DD, finishProcessingMsg);
        EV << "***************************" << endl;
        setNextWakeUpAfterTime(processingAndWaitTime.dbl());
        return;
    }

    EV << "***************************" << endl;
    // send(frame, "outN");
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

void Node::send_frame(frame_kind fk, seq_nr frame_nr, seq_nr frame_expected, vector<Packet> &buffer, string simulationParams, bool isAgain) {
    Frame_Base *frame;
    if (fk == DATA) {
        frame = create_frame(buffer[frame_nr % NR_BUFS], frame_nr);
        start_timer(frame_nr);
    } else {
        frame = new Frame_Base("");
        frame->setM_Type(fk);
        frame->setAck(frame_expected);
        if (fk == NACK) {
            no_nack = false;
        }
    }

    to_physical_layer(frame, simulationParams, isAgain);  // added frame_nr
}

Frame_Base *Node::create_frame(string payload, seq_nr frame_nr) {
    Frame_Base *frame = new Frame_Base("");

    bitset<8> checkSum = check_sum(payload);
    frame->setMycheckbits(checkSum);

    frame->setM_Payload(payload.c_str());
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

string Node::byte_destuffing(string str) {
    string result = "";
    for (int i = 1; i < str.size() - 1; i++) {
        if (str[i] == ESCAPE_BYTE) {
            i++;
        }
        result += str[i];
    }
    return result;
}

void Node::clearFile(const std::string &filename) {
    std::ofstream file;
    file.open(filename, std::ios::out | std::ios::trunc);
    file.close();
}
void Node::writeStartingToOutputFile(Frame_Base *frame, string simulationParams) {
    if (type == RECEIVER || frame->getM_Type() != DATA) return;
    ofstream file;
    file.open(OUTPUT_FILE, ios::out | ios::app);

    if (!file.is_open()) {
        cerr << "Error opening file: " << strerror(errno) << endl;
    } else {
        file << "At Time " << simTime() << ", " << getName() << " Introducing channel error with code = " << simulationParams << endl;
    }
    file.close();
}

void Node::writeStringToFile(string msg) {
    ofstream file;
    file.open(OUTPUT_FILE, ios::out | ios::app);
    if (!file.is_open()) {
        cerr << "Error opening file: " << strerror(errno) << endl;
    } else {
        file << msg << endl;
    }
    file.close();
}

string Node::getTransmissionToOutputFileMsg(Frame_Base *frame, int modified, bool lost, int duplicate, bool delay) {
    if (type == RECEIVER || frame->getM_Type() != DATA) return "";
    stringstream ss;
    ss << "At Time " << (duplicate == 2 ? simTime() + PT + DD : simTime() + PT) << ", " << getName() << "[sender][sent] frame with seq_num = " << frame->getSeq_Num() << " and payload=" << frame->getM_Payload() << " and trailer=" << frame->getMycheckbits() << ", Modified="
       << modified << ", Lost=" << (lost ? "YES" : "NO") << ", Duplicate=" << duplicate << ", Delay=" << (delay ? ED : 0) << endl
       << endl;
    return ss.str();
}
void Node::writeRecievedToOutputFile(Frame_Base *frame, bool modified, int duplicate, string payload) {
    if (type == SENDER || frame->getM_Type() != DATA) return;
    ofstream file;
    file.open(OUTPUT_FILE, ios::out | ios::app);

    if (!file.is_open()) {
        cerr << "Error opening file: " << strerror(errno) << endl;
    } else {
        file << "At Time " << simTime() << ", " << getName() << "[reciever][recieved] frame with seq_num = " << frame->getSeq_Num() << " and payload=" << payload << " and trailer=" << frame->getMycheckbits() << ", Modified="
             << (modified ? "yes" : "-1") << ", Duplicate=" << duplicate << endl
             << endl;
    }
    file.close();
}

void Node::writeToTimeOutFile(int seqNumber) {
    ofstream file;
    file.open(OUTPUT_FILE, ios::out | ios::app);

    if (!file.is_open()) {
        cerr << "Error opening file: " << strerror(errno) << endl;
    } else {
        file << "Time out event at time " << simTime() << ",at " << getName() << " for frame with seq_num = " << seqNumber << endl;
    }
    file.close();
}

string Node::getAckToOutputFileMsg(Frame_Base *frame) {
    seq_nr seqNum = frame->getAck();
    if (type == SENDER) return "";
    stringstream ss;

    ss << "At Time " << simTime() + PT << ", " << getName() << " Sending ACK with number = "
       << seqNum << ", Loss "
       << "No" << endl;

    return ss.str();
}

string Node::getNackToOutputFileMsg(Frame_Base *frame, bool isLoss) {
    seq_nr seqNum = frame->getSeq_Num();
    if (type == SENDER) return "";
    stringstream ss;
    ss << "At Time " << simTime() + PT << ", " << getName() << " Sending NACK with number = "
       << seqNum << ", Loss " << (isLoss == 1 ? "Yes" : "No") << endl;

    return ss.str();
}
