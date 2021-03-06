//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2005 Wei Yang, Ng
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
// Edited with OMNeT++ Academic/Noncommercial Edition.
//
package inet.networklayer.ipv6;

import inet.common.MessageDispatcher;
import inet.networklayer.contract.IIPv6Tunneling;
import inet.networklayer.contract.INetworkLayer;
import inet.networklayer.contract.IxMIPv6Support;
import inet.networklayer.icmpv6.ICMPv6;
import inet.networklayer.icmpv6.IPv6NeighbourDiscovery;

//
// Represents an IPv6 network layer (L3).
//
// The module has ports to connect to a higher layer (TCP, UDP) and
// several network interfaces.
//
module IPv6NetworkLayer like INetworkLayer
{
    parameters:
        bool xMIPv6Support = default(false);
        bool forwarding = default(false);
        bool multicastForwarding = default(false);
        *.xmipv6Module = xMIPv6Support ? absPath(".mipv6support.xMobileIPv6") : "";
        string interfaceTableModule;
        *.forwarding = forwarding;
        *.multicastForwarding = multicastForwarding;
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.routingTableModule = default(absPath(".routingTable"));
        *.ipv6NeighbourDiscoveryModule = default(absPath(".neighbourDiscovery"));
        *.icmpv6Module = default(absPath(".icmpv6"));
        *.ipv6TunnelingModule = default(absPath(".iptunneling"));
        @display("i=block/fork");

    gates:
        input ifIn @labels(INetworkDatagram);
        output ifOut @labels(INetworkDatagram);
        input transportIn @labels(IPv4ControlInfo/down);
        output transportOut @labels(IPv4ControlInfo/up);

    submodules:
        routingTable: IPv6RoutingTable {
            parameters:
                @display("p=100,100;is=s");
        }
        up: MessageDispatcher {
            parameters:
                @display("p=450,100;b=480,5");
        }
        icmpv6: ICMPv6 {
            parameters:
                @display("p=600,200");
        }
        lp: MessageDispatcher {
            parameters:
                @display("p=450,300;b=480,5");
        }
        neighbourDiscovery: IPv6NeighbourDiscovery {
            parameters:
                @display("p=300,400");
        }
        // TODO: rename to ip
        ipv6: IPv6 {
            parameters:
                @display("p=450,400");
        }
        iptunneling: <default(firstAvailable("IPv6Tunneling"))> like IIPv6Tunneling {
            parameters:
                @display("p=600,400");
        }
        mipv6support: <default("xMIPv6Support")> like IxMIPv6Support if xMIPv6Support {
            parameters:
                @display("p=600,500");
        }

    connections allowunconnected:  // FIXME remove 'nocheck'!
        transportIn --> { @display("m=n"); } --> up.upperLayerIn++;
        transportOut <-- { @display("m=n"); } <-- up.upperLayerOut++;

        up.lowerLayerOut++ --> lp.upperLayerIn++;
        up.lowerLayerIn++ <-- lp.upperLayerOut++;

        up.lowerLayerOut++ --> icmpv6.transportIn;
        up.lowerLayerIn++ <-- icmpv6.transportOut;

        icmpv6.ipv6Out --> lp.upperLayerIn++;
        icmpv6.ipv6In <-- lp.upperLayerOut++;

        lp.lowerLayerOut++ --> ipv6.transportIn;
        lp.lowerLayerIn++ <-- ipv6.transportOut;

        iptunneling.upperLayerOut --> ipv6.upperTunnelingIn;
        iptunneling.upperLayerIn <-- ipv6.upperTunnelingOut;

        iptunneling.linkLayerOut --> ipv6.lowerTunnelingIn;
        iptunneling.linkLayerIn <-- ipv6.lowerTunnelingOut;

        neighbourDiscovery.ipv6Out --> ipv6.ndIn;
        neighbourDiscovery.ipv6In <-- ipv6.ndOut;

        mipv6support.fromIPv6 <-- ipv6.xMIPv6Out if xMIPv6Support;
        mipv6support.toIPv6 --> ipv6.xMIPv6In if xMIPv6Support;

        ipv6.queueOut --> { @display("m=s"); } --> ifOut;
        ipv6.queueIn <-- { @display("m=s"); } <-- ifIn;
}
