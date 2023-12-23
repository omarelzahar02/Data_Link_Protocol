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

#include "Coordinator.h"

#include "./initMsg_m.h"
#include "./protocol.h"
#define COORDINATOR_FILE "./coordinator.txt"
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
            printf("%s\n", line.c_str());
        }
    }
    file.close();
    std::cout << lines[0] << std::endl;
    int startingNode = lines[0][0] - '0';
    timing startingTime = std::stoi(lines[0].substr(2));
    InitMsg_Base *msgToSender = new InitMsg_Base("");
    msgToSender->setStartTime(startingTime);
    msgToSender->setType(SENDER);
    send(msgToSender, "outNode0");
    InitMsg_Base *msgToReceiver = new InitMsg_Base("");
    msgToReceiver->setType(RECEIVER);
    send(msgToReceiver, "outNode1");
}

void Coordinator::handleMessage(cMessage *msg) {
    // No messages are expected to be received by the Coordinator
}
