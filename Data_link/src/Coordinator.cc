#include "Coordinator.h"

#include "./initMsg_m.h"
#include "./protocol.h"
#define COORDINATOR_FILE "../src/coordinator.txt"
using namespace std;
Define_Module(Coordinator);

void Coordinator::initialize() {
    // TODO - Generated method body
    std::ifstream file;
    std::vector<std::string> lines;

    std::string filename = COORDINATOR_FILE;
    file.open(filename);
    if (!file.is_open()) {
        std::cout << "Error opening coordinator file" << std::endl;
    } else {
        std::string line;
        while (getline(file, line)) {
            lines.push_back(line);
            std::cout << line << std::endl;
        }
    }
    file.close();
    std::cout << lines[0] << std::endl;
    int startingNode = lines[0][0] - '0';
    timing startingTime = std::stoi(lines[0].substr(2));
    InitMsg_Base *msgToSender = new InitMsg_Base("");
    msgToSender->setStartTime(startingTime);
    msgToSender->setType(SENDER);
    string senderNodePort;
    string receiverNodePort;
    if (startingNode == 0) {
        senderNodePort = "outNode0";
        receiverNodePort = "outNode1";
    } else {
        senderNodePort = "outNode1";
        receiverNodePort = "outNode0";
    }
    send(msgToSender, senderNodePort.c_str());
    InitMsg_Base *msgToReceiver = new InitMsg_Base("");
    msgToReceiver->setType(RECEIVER);
    send(msgToReceiver, receiverNodePort.c_str());
}

void Coordinator::handleMessage(cMessage *msg) {
    // No messages are expected to be received by the Coordinator
}
