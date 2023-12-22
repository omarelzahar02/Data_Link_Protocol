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

Define_Module(Coordinator);

void Coordinator::initialize()
{
    // TODO - Generated method body
    std::ifstream file;
    std::vector<std::string> lines;

    file.open("input0.txt");
    if(!file.is_open())
    {
        std::cout<<"Error opening file"<<std::endl;
    }
    else
    {
        std::string line;
        while(getline(file,line))
        {
            lines.push_back(line);
        }
        std::cout<< lines.size()<<std::endl;
    }
    file.close();
}

void Coordinator::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}
