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
import inet.networklayer.mpls.LIBTable;
import inet.networklayer.mpls.MPLS;
import inet.networklayer.rsvp_te.RSVP;
import inet.networklayer.rsvp_te.SimpleClassifier;
import inet.networklayer.ted.LinkStateRouting;
import inet.networklayer.ted.TED;


//
// An RSVP-TE capable router.
//
// RSVP occupies the Transport layer; however, it is not a transport protocol
// itself. RSVP uses transport protocols to route packets. ~TED is used
// to calculate shortest paths.
//
module RSVP_LSR
{
    parameters:
        @networkNode();
        @labels(node,mpls-node);
        @display("i=abstract/router");
        bool hasStatus = default(false);
        int numLoInterfaces = default(1);
        string peers;
        string routerId = default("auto");
        *.forwarding = true;
        *.routingTable.routerId = routerId;
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(absPath(".ipv4.routingTable"));
        *.tedModule = default(absPath(".ted"));
        *.rsvpModule = default(absPath(".rsvp"));
        *.libTableModule = default(absPath(".libTable"));
    gates:
        inout pppg[] @labels(PPPFrame-conn);
    submodules:
        status: NodeStatus if hasStatus {
            @display("p=58,50;is=s");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=58,242;is=s");
        }
        ted: TED {
            parameters:
                @display("p=312,203");
        }
        linkStateRouting: LinkStateRouting {
            parameters:
                peers = peers;
                @display("p=254,115");
        }
        rsvp: RSVP {
            parameters:
                peers = peers;
                classifierModule = "^.classifier";
                @display("p=156,67");
        }
        classifier: SimpleClassifier {
            parameters:
                @display("p=330,67");
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
            @display("p=139,394");
        }
        ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=132,345,row,90;q=l2queue");
        }
        mpls: MPLS {
            parameters:
                classifierModule = "^.classifier";
                @display("p=448,311");
        }
        libTable: LIBTable {
            parameters:
                @display("p=312,268");
        }
        ml: MessageDispatcher {
            parameters:
                @display("p=409,350;b=580,5,,,,1");
        }
    connections allowunconnected:
        linkStateRouting.ipOut --> tn.upperLayerIn++;
        tn.upperLayerOut++ --> linkStateRouting.ipIn;
        ipv4.transportOut --> tn.lowerLayerIn++;
        tn.lowerLayerOut++ --> ipv4.transportIn;

        rsvp.ipOut --> tn.upperLayerIn++;
        rsvp.ipIn <-- tn.upperLayerOut++;

        ipv4.ifOut --> nm.upperLayerIn++;
        nm.upperLayerOut++ --> ipv4.ifIn;

        for i=0..numLoInterfaces-1 {
            lo[i].upperLayerOut --> nm.lowerLayerIn++;
            nm.lowerLayerOut++ --> lo[i].upperLayerIn;
        }

        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;

            ppp[i].upperLayerOut --> ml.lowerLayerIn++;
            ml.lowerLayerOut++ --> ppp[i].upperLayerIn;
        }
        mpls.ifOut --> ml.upperLayerIn++;
        ml.upperLayerOut++ --> mpls.ifIn;
        nm.lowerLayerOut++ --> mpls.netwIn;
        mpls.netwOut --> nm.lowerLayerIn++;
}

