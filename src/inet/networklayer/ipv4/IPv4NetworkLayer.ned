//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.networklayer.ipv4;

import inet.common.MessageDispatcher;
import inet.networklayer.contract.IARP;
import inet.networklayer.configurator.ipv4.IPv4NodeConfigurator;
import inet.networklayer.contract.INetworkLayer;


//
// Network layer of an IPv4 node.
//
// Interfaces to transport layer: TCP, UDP, echo/ping, RSVP
//
module IPv4NetworkLayer like INetworkLayer
{
    parameters:
        bool proxyARP = default(true);
        bool forwarding = default(false);
        bool multicastForwarding = default(false);
        string igmpType = default("IGMPv2");
        string arpType = default("ARP");
        string interfaceTableModule;
        *.forwarding = forwarding;
        *.multicastForwarding = multicastForwarding;
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.routingTableModule = default(absPath(".routingTable"));
        *.arpModule = default(absPath(".arp"));
        *.icmpModule = default(absPath(".icmp"));
        arp.respondToProxyARP = proxyARP;
        @display("i=block/fork");

    gates:
        input ifIn @labels(INetworkDatagram);
        output ifOut @labels(INetworkDatagram);
        input transportIn @labels(IPv4ControlInfo/down);
        output transportOut @labels(IPv4ControlInfo/up);
        input igmpIn;
        output igmpOut;

    submodules:
        configurator: IPv4NodeConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        routingTable: IPv4RoutingTable {
            parameters:
                @display("p=100,200;is=s");
        }
        up: MessageDispatcher {
            parameters:
                @display("p=450,100;b=480,5");
        }
        igmp: <igmpType> like IIGMP {
            parameters:
                @display("p=300,200");
        }
        icmp: ICMP {
            parameters:
                @display("p=600,200");
        }
        mp: MessageDispatcher {
            parameters:
                @display("p=450,300;b=480,5");
        }
        ip: IPv4 {
            parameters:
                useProxyARP = true; // as routes with unspecified next-hop addr are quite common
                @display("p=450,400;q=queue");
        }
        arp: <arpType> like IARP {
            parameters:
                @display("p=300,400;q=pendingQueue");
        }
        lp: MessageDispatcher {
            parameters:
                @display("p=450,500;b=480,5");
        }

    connections allowunconnected:
        transportIn --> { @display("m=n"); } --> up.upperLayerIn++;
        transportOut <-- { @display("m=n"); } <-- up.upperLayerOut++;

        up.lowerLayerOut++ --> igmp.routerIn;
        up.lowerLayerIn++ <-- igmp.routerOut;

        up.lowerLayerOut++ --> mp.upperLayerIn++;
        up.lowerLayerIn++ <-- mp.upperLayerOut++;

        up.lowerLayerOut++ --> icmp.transportIn;
        up.lowerLayerIn++ <-- icmp.transportOut;

        igmp.ipOut --> mp.upperLayerIn++;
        igmp.ipIn <-- mp.upperLayerOut++;

        icmp.ipOut --> mp.upperLayerIn++;
        icmp.ipIn <-- mp.upperLayerOut++;

        mp.lowerLayerOut++ --> ip.transportIn;
        mp.lowerLayerIn++ <-- ip.transportOut;

        arp.ifOut --> lp.upperLayerIn++;
        arp.ifIn <-- lp.upperLayerOut++;

        ip.queueOut --> lp.upperLayerIn++;
        ip.queueIn <-- lp.upperLayerOut++;

        lp.lowerLayerOut++ --> { @display("m=s"); } --> ifOut;
        lp.lowerLayerIn++ <-- { @display("m=s"); } <-- ifIn;
}
