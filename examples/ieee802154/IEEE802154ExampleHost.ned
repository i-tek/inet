//
// Copyright (C) 2016 Florian Kauer <florian.meier@koalo.de>
// Copyright (C) 2016 Jonas K. <i-tek@web.de>
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

package inet.examples.ieee802154;


import inet.node.inet.WirelessHost;
import inet.linklayer.ieee802154.Ieee802154NarrowbandNic;
import inet.routing.gpsr.GPSR;

import inet.applications.contract.IIPApp;

//
// A wireless host containing an Ieee802154NarrowbandNic,
// GPSR and a IPvXTrafGen.
//
module IEEE802154ExampleHost extends WirelessHost
{
    parameters:
        int numIpApps = default(0);
        hasIpv4 = false;
        hasGn = true;
        networkLayerType = "GenericNetworkLayer";
        forwarding = default(true);

        wlan[0].typename = default("WirelessNic");
        wlan[0].radioType = default("Ieee802154NarrowbandScalarRadio");

        @display("i=device/accesspoint;bgb=,-");

        *.routingTableModule = "^.generic.routingTable";
        *.networkProtocolModule = "^.generic.gnp";

        *.gnp.procDelay = default(2ms);

    submodules:

        // IPLevel Apps
        ipApp[numIpApps]: <> like IIPApp {
            parameters:
                @display("p=300,300,row,100");
        }

	//Routing protocol
        gpsr: GPSR {
            @display("p=739,339");
        }

    connections:

        for i=0..numIpApps-1 {
            ipApp[i].ipOut --> tn.upperLayerIn++;
            ipApp[i].ipIn <-- tn.upperLayerOut++;
        }

        gpsr.ipOut --> tn.upperLayerIn++;
        gpsr.ipIn <-- tn.upperLayerOut++;
}
