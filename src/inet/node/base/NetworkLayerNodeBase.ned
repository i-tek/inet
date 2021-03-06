//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.node.base;

import inet.common.MessageDispatcher;
import inet.networklayer.contract.INetworkLayer;
import inet.networklayer.generic.GenericNetworkLayer;
import inet.networklayer.ipv4.IPv4NetworkLayer;
import inet.networklayer.ipv6.IPv6NetworkLayer;

module NetworkLayerNodeBase extends LinkLayerNodeBase
{
    parameters:
        bool hasIpv4 = default(true);
        bool hasIpv6 = default(false);
        bool hasGn = default(false);    //FIXME redundant/unused? !!! see also networkLayerType, 
        string networkLayerType = default("");
        bool forwarding = default(false);
        bool multicastForwarding = default(false);
        *.forwarding = forwarding;
        *.multicastForwarding = multicastForwarding;
        @figure[networkLayer](type=rectangle; pos=250,400; size=680,200; fillColor=#00ff00; cornerRadius=12; fillOpacity=0.1);
        @figure[networkLayer.title](type=text; pos=800,410; text="network layer");

    submodules:
        ipv4: IPv4NetworkLayer if hasIpv4 {
            parameters:
                @display("p=400,500;q=queue");
        }
        ipv6: IPv6NetworkLayer if hasIpv6 {
            parameters:
                @display("p=500,500;q=queue");
        }
        generic: <networkLayerType> like INetworkLayer if networkLayerType != "" {
            parameters:
                @display("p=600,500;q=queue");
        }
        nl: MessageDispatcher {
            parameters:
                @display("p=550,600;b=580,5,,,,1");
        }

    connections allowunconnected:
        ipv4.ifIn <-- nl.upperLayerOut++ if hasIpv4;
        ipv4.ifOut --> nl.upperLayerIn++ if hasIpv4;

        ipv6.ifIn <-- nl.upperLayerOut++ if hasIpv6;
        ipv6.ifOut --> nl.upperLayerIn++ if hasIpv6;

        generic.ifIn <-- nl.upperLayerOut++ if networkLayerType != "";
        generic.ifOut --> nl.upperLayerIn++ if networkLayerType != "";

        for i=0..numLoInterfaces-1 {
            nl.lowerLayerOut++ --> lo[i].upperLayerIn;
            lo[i].upperLayerOut --> nl.lowerLayerIn++;
        }

        for i=0..sizeof(radioIn)-1 {
            wlan[i].upperLayerOut --> nl.lowerLayerIn++;
            wlan[i].upperLayerIn <-- nl.lowerLayerOut++;
        }

        for i=0..sizeof(ethg)-1 {
            eth[i].upperLayerOut --> nl.lowerLayerIn++;
            eth[i].upperLayerIn <-- nl.lowerLayerOut++;
        }

        for i=0..sizeof(pppg)-1 {
            ppp[i].upperLayerOut --> nl.lowerLayerIn++;
            ppp[i].upperLayerIn <-- nl.lowerLayerOut++;
        }

        for i=0..numExtInterfaces-1 {
            ext[i].upperLayerOut --> nl.lowerLayerIn++;
            ext[i].upperLayerIn <-- nl.lowerLayerOut++;
        }

        for i=0..numTunInterfaces-1 {
            tun[i].upperLayerOut --> nl.lowerLayerIn++;
            tun[i].upperLayerIn <-- nl.lowerLayerOut++;
        }
}
