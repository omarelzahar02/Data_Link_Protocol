#ifndef __DATA_LINK_NODE_H_
#define __DATA_LINK_NODE_H_

#include <omnetpp.h>

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include "frame_m.h"
#include "initMsg_m.h"
#include "protocol.h"
#include "selfMsg_m.h"

using namespace omnetpp;
using namespace std;
class Node : public cSimpleModule {
    // public:
    // bool timeToPrint = false;
   private:
    /*Simulation Variables*/
    int NR_BUFS;
    int WS;     // Sender window size
    int WR;     // Receiver window size
    double TO;  // Timeout
    double PT;  // Processing time
    double TD;  // Transmission delay
    double ED;  // Error delay
    double DD;  // Duplication delay
    timing startingTime;
    queue<string> data;
    queue<string> status;
    node_type type;
    SelfMsg_Base *wakeUpMsg;

    /*Protocol Variables*/
    unordered_map<seq_nr, cMessage *> timers;
    seq_nr next_frame_to_send;
    seq_nr ack_expected;
    seq_nr frame_expected;
    seq_nr too_far;
    seq_nr nbuffered;
    vector<Packet> inbuffer;
    vector<Packet> outbuffer;
    vector<bool> arrived;
    seq_nr oldest_frame;
    bool is_network_layer_enabled;
    bool no_nack;

    simtime_t lstProcessingFinish = 0;
    bool isFirstTime = true;

    void setNodeType(node_type type);
    void handleCoordiantionMessage(InitMsg_Base *msg);
    void handleFrameMessage(Frame_Base *msg);
    void setNodeStartingTime(timing startingTime);
    void initNode(node_type type, timing startingTime = 0);
    void sendSelfMsg(double sleepDuration, int msgType, int seqNumber = -1);
    void readFile();
    void terminate();
    void handleTimerTimeOut(int seqNumber);
    void handleReceivingAck(Frame_Base *received_msg);
    void handleReceivingData(Frame_Base *received_msg);
    void handleReceivingNack(Frame_Base *received_msg);
    bool is_received_data_correct(Frame_Base *s);
    void check_for_network_layer_event(void);
    bool checkForProcessingFinishToDisplay(cMessage *msg);
    void writeStringToFile(string msg);
    string getTransmissionToOutputFileMsg(Frame_Base *frame, int modified, bool lost, int duplicate, bool delay);
    bool between(seq_nr a, seq_nr b, seq_nr c);
    bool from_network_layer(Packet &p, string &simulationParams);
    void to_network_layer(Packet *p);
    void to_physical_layer(Frame_Base *frame, string simulationParams = "0000", bool isAgain = 0);
    void setNextWakeUpAfterTime(double time);
    string modifyRandomBit(string s, int beg, int end, int &randPos);
    void start_timer(seq_nr seqNum);
    void stop_timer(seq_nr seqNum);
    void enable_network_layer(void);
    void disable_network_layer(void);
    bool checkForDelayedMsgsToSend(cMessage *msg);
    void sendAfter(Frame_Base *frame, double delay, double processingMsgDelay, string processingMsg = "");
    void send_frame(frame_kind fk, seq_nr frame_nr, seq_nr frame_expected, vector<Packet> &buffer, string simulationParams = "0000", bool isAgain = 0);
    bitset<8> check_sum(string payload);
    void simulate_sending(int Modification, int Loss, int Duplication, int Delay);
    string byte_stuffing(string str);
    string byte_destuffing(string str);
    Frame_Base *create_frame(string payload, seq_nr frame_nr);
    void clearFile(const std::string &filename);
    void writeStartingToOutputFile(Frame_Base *frame, string simulationParams = "0000");
    void scheduleProcessingMessage(string processingMsg, double processingMsgDelay);
    //    void writeTransmitToOutputFile(Frame_Base *frame, string simulationParams = "0000");
    void writeRecievedToOutputFile(Frame_Base *frame, bool modified, int duplicate, string payload);
    void writeToTimeOutFile(seq_nr seqNum);
    string getAckToOutputFileMsg(Frame_Base *frame);
    string getNackToOutputFileMsg(Frame_Base *frame, bool isLoss);

   protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

#endif
