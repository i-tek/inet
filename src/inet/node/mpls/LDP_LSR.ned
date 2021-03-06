//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
package inet.node.mpls;

import inet.common.MessageDispatcher;
import inet.common.lifecycle.NodeStatus;
import inet.linklayer.contract.IWiredNic;
import inet.linklayer.loopback.LoopbackInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.IPv4NetworkLayer;
import inet.networklayer.ipv4.IPv4RoutingTable;
import inet.networklayer.ldp.LDP;
import inet.networklayer.mpls.LIBTable;
import inet.networklayer.mpls.MPLS;
import inet.networklayer.ted.LinkStateRouting;
import inet.networklayer.ted.TED;
import inet.transportlayer.contract.ITCP;
import inet.transportlayer.contract.IUDP;


//
// An LDP-capable router.
//
// LDP Capable Routers are the main building blocks for an LDP MPLS network.
//
module LDP_LSR
{
    parameters:
        @networkNode();
        @labels(node);
        @display("i=abstract/router");
        bool hasStatus = default(false);
        double holdTime @unit("s") = default(15s);
        double helloInterval @unit("s") = default(5s);
        int numLoInterfaces = default(1);
        string peers;
        string routerId = default("auto");
        string tcpType = default(firstAvailable("TCP","TCP_lwIP","TCP_NSC"));  // tcp implementation (e.g. ~TCP, ~TCP_lwIP, ~TCP_NSC) or ~TCPSpoof
        *.forwarding = true;
        *.routingTable.routerId = routerId;
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(absPath(".ipv4.routingTable"));
        *.tedModule = default(absPath(".ted"));
        *.libTableModule = default(absPath(".libTable"));
    gates:
        inout pppg[] @labels(PPPFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            @display("p=73,45;is=s");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=73,176;is=s");
        }
        ldp: LDP {
            parameters:
                holdTime = holdTime;
                helloInterval = helloInterval;
                @display("p=222,67");
        }
        ted: TED {
            parameters:
                @display("p=340,244");
        }
        linkStateRouting: LinkStateRouting {
            parameters:
                peers = peers;
                @display("p=360,150");
        }
        tcp: <tcpType> like ITCP {
            parameters:
                @display("p=177,145");
        }
        udp: <default(firstAvailable("UDP"))> like IUDP {
            parameters:
                @display("p=267,145");
        }
        tn: MessageDispatcher {
            parameters:
                @display("p=409,185;b=580,5,,,,1");
        }
        ipv4: IPv4NetworkLayer {
            parameters:
                @display("p=409,223");
        }
        nm: MessageDispatcher {
            parameters:
                @display("p=409,271;b=580,5,,,,1");
        }
        lo[numLoInterfaces]: LoopbackInterface {
            @display("p=50,388");
        }
        ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=131,388,row,90;q=l2queue");
        }
        mpls: MPLS {
            parameters:
                //peers = peers,
                classifierModule = "^.ldp";
                @display("p=401,309");
        }
        libTable: LIBTable {
            parameters:
                @display("p=671,302");
        }

        ml: MessageDispatcher {
            parameters:
                @display("p=409,350;b=580,5,,,,1");
        }
    connections allowunconnected:
        linkStateRouting.ipOut --> tn.upperLayerIn++;
        tn.upperLayerOut++ --> linkStateRouting.ipIn;

        ldp.tcpIn <-- tcp.appOut;
        ldp.tcpOut --> tcp.appIn;

        ldp.udpOut --> udp.appIn;
        ldp.udpIn <-- udp.appOut;

        udp.ipOut --> tn.upperLayerIn++;
        tn.upperLayerOut++ --> udp.ipIn;

        tcp.ipOut --> tn.upperLayerIn++;
        tn.upperLayerOut++ --> tcp.ipIn;

        for i=0..numLoInterfaces-1 {
            lo[i].upperLayerOut --> nm.lowerLayerIn++;
            nm.lowerLayerOut++ --> lo[i].upperLayerIn;
        }

        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;

            ppp[i].upperLayerOut --> ml.lowerLayerIn++;
            ml.lowerLayerOut++ --> ppp[i].upperLayerIn;

        }
        ipv4.transportOut --> tn.lowerLayerIn++;
        tn.lowerLayerOut++ --> ipv4.transportIn;
        ipv4.ifOut --> nm.upperLayerIn++;
        nm.upperLayerOut++ --> ipv4.ifIn;
        nm.lowerLayerOut++ --> mpls.netwIn;
        mpls.netwOut --> nm.lowerLayerIn++;
        mpls.ifOut --> ml.upperLayerIn++;
        ml.upperLayerOut++ --> mpls.ifIn;
}
