package data_link;

@license(LGPL);
//
// TODO documentation
//
network Network
{
    parameters:
        int WS = default(3);         // Sender window size
        int WR = default(3);         // Receiver window size
        int TO = default(10);         // Timeout
        double PT = default(0.5);         // Processing time
        double TD = default(1.0);         // Transmission delay
        double ED = default(4.0);         // Error delay
        double DD = default(0.1);         // Duplication delay
        @display("bgb=634,365");
    submodules:
        Node0: Node {
            @display("p=110,86;i=device/wifilaptop");
        }
        Node1: Node {
            @display("p=445,108;i=old/laptop3");
        }
        Coordinator: Coordinator {
            @display("p=260,235;i=block/cogwheel");
        }
    connections:
        Coordinator.outNode0 --> Node0.inC;
        Coordinator.outNode1 --> Node1.inC;
        Node0.outN --> Node1.inN;
        Node1.outN --> Node0.inN;
}

