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


bool between(seq_nr a, seq_nr b, seq_nr c)                          // maybe function will be needed to be static
{
    if (((a <= b) && (b < c)) || ((c < a) && (a <= b)) || ((b < c) && (c < a)))
        return true;
    else
        return false;
}

void send_data(frame_kind fk, seq_nr frame_nr, seq_nr frame_expected, packet buffer[])
{
    Frame_Base s;
    s.kind = fk;
    if(fk == data)
    {
        s.info = buffer[frame_nr % NR_BUFS];
    }
    s.seq = frame_nr;
    s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1);
    if(fk == nak)
    {
        no_ack = false;
    }
    to_physical_layer(&s);                                                      //function needs to be implemented
    if(fk == data)
    {
        start_timer(frame_nr % NR_BUFS);                                        //function needs to be implemented
    }
    stop_ack_timer();                                                           //function needs to be implemented
}

Define_Module(Node);



void Node::initialize()
{
    // TODO - Generated method body
    std::ifstream file;
    std::vector<std::string> lines;
    std::vector<std::string> data;
    std::vector<std::string> status;
    if(strcmp(getName(),"Node0") == 0)
        file.open("input0.txt");
    else
        file.open("input1.txt");
    if (!file.is_open())
    {
        std::cout << "Error opening file" << std::endl;
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

        // std::cout<<status[i]<<std::endl;
        // std::cout<<data[i]<<std::endl;
    }
}

void Node::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}
