//
// Copyright (C) 2013 OpenSim Ltd.
// Copyright (C) 2016 Jonas K. <i-tek@web.de>
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

#ifndef __INET_PROTOCOL_H
#define __INET_PROTOCOL_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API Protocol
{
  protected:
    static int nextId;
    static std::map<int, const Protocol *> idToProtocol;
    static std::map<std::string, const Protocol *> nameToProtocol;

  protected:
    const int id;
    const char *name;

  public:
    Protocol(const char *name);

    const int getId() const { return id; }
    const char *getName() const { return name; }

    static const Protocol *findProtocol(int id);
    static const Protocol *getProtocol(int id);

    static const Protocol *findProtocol(const char *name);
    static const Protocol *getProtocol(const char *name);

  public:
    // in alphanumeric order
    static const Protocol aodv;
    static const Protocol arp;
    static const Protocol bgp;
    static const Protocol dsdv2;
    static const Protocol dymo;
    static const Protocol ethernet;
    static const Protocol gpsr;
    static const Protocol ieee80211;

    // ip protocols (in order of ProtocolGroup)
    static const Protocol hopoptv6;
    static const Protocol icmpv4; // implemented
    static const Protocol igmp; // implemented
    static const Protocol ggp;
    static const Protocol ipv4; // implemented
    static const Protocol st;
    static const Protocol tcp; // implemented
    static const Protocol cbt;
    static const Protocol egp; // implemented
    static const Protocol igp; // implemented
    static const Protocol bbnrccmon;
    static const Protocol nvp2;
    static const Protocol pup;
    static const Protocol argus; // deprecated
    static const Protocol emcon;
    static const Protocol xnet;
    static const Protocol chaos;
    static const Protocol udp; // implemented
    static const Protocol mux;
    static const Protocol dcn;
    static const Protocol hmp;
    static const Protocol prm;
    static const Protocol xnsidp;
    static const Protocol trunk1;
    static const Protocol trunk2;
    static const Protocol leaf1;
    static const Protocol leaf2;
    static const Protocol rdp;
    static const Protocol irtp;
    static const Protocol isotp4;
    static const Protocol netblt;
    static const Protocol mfe;
    static const Protocol merit;
    static const Protocol dccp;
    static const Protocol tpcp;
    static const Protocol idpr;
    static const Protocol xtp; // implemented
    static const Protocol ddp;
    static const Protocol idprcmtp;
    static const Protocol tppp;
    static const Protocol il;
    static const Protocol ipv6; // implemented
    static const Protocol sdrp;
    static const Protocol routev6;
    static const Protocol fragv6;
    static const Protocol idrp;
    static const Protocol rsvp; // implemented
    static const Protocol gre;
    static const Protocol dsr; // implemented
    static const Protocol bna;
    static const Protocol esp;
    static const Protocol ah;
    static const Protocol inlsp;
    static const Protocol swipe; // deprecated
    static const Protocol narp;
    static const Protocol mobilev4;
    static const Protocol tlsp;
    static const Protocol skip;
    static const Protocol icmpv6; // implemented
    static const Protocol nonxtv6;
    static const Protocol optsv6;
    static const Protocol ahint;  // Any host internal protocol
    static const Protocol cftp;
    static const Protocol alan;  // Any local network
    static const Protocol satexpak;
    static const Protocol kryptolan;
    static const Protocol rvd;
    static const Protocol ippc;
    static const Protocol distfile; // Any distributed file system
    static const Protocol satmon;
    static const Protocol visa;
    static const Protocol ipcv;
    static const Protocol cpnx;
    static const Protocol cphb;
    static const Protocol wsn;
    static const Protocol pvp;
    static const Protocol brsatmon;
    static const Protocol sunnd;
    static const Protocol wbmon;
    static const Protocol wbexpak;
    static const Protocol isoip;
    static const Protocol vmtp;
    static const Protocol svmtp;
    static const Protocol vines;
    static const Protocol ttp; // Second definition: { 84, &Protocol::iptm },
    static const Protocol nsfnet;
    static const Protocol dgp;
    static const Protocol tcf;
    static const Protocol eigrp;
    static const Protocol ospf; // implemented
    static const Protocol sprite;
    static const Protocol larp;
    static const Protocol mtp;
    static const Protocol ax25;
    static const Protocol ipip;
    static const Protocol micp; // deprecated
    static const Protocol sccsp;
    static const Protocol etherip;
    static const Protocol encap;
    static const Protocol privenc; // Custom private encryption scheme
    static const Protocol gmtp;
    static const Protocol ifmp;
    static const Protocol pnni;
    static const Protocol pim; // implemented
    static const Protocol aris;
    static const Protocol scps;
    static const Protocol qnx;
    static const Protocol an;
    static const Protocol ipcomp;
    static const Protocol snp;
    static const Protocol compaq;
    static const Protocol ipxip;
    static const Protocol vrrp;
    static const Protocol pgm;
    static const Protocol zerohop; // Custom 0-hop protocols
    static const Protocol l2tp;
    static const Protocol ddx;
    static const Protocol iatp;
    static const Protocol stp;
    static const Protocol srp;
    static const Protocol uti;
    static const Protocol smp;
    static const Protocol sm;   // deprecated
    static const Protocol ptp;
    static const Protocol isisv4;
    static const Protocol fire;
    static const Protocol crtp;
    static const Protocol crudp;
    static const Protocol sscopmce;
    static const Protocol iplt;
    static const Protocol sps;
    static const Protocol pipe;
    static const Protocol sctp; // implemented
    static const Protocol fc;
    static const Protocol rsvpignore;
    static const Protocol mobilityv6;
    static const Protocol udplite;
    static const Protocol mplsip;
    static const Protocol manet; // implemented
    static const Protocol hip;
    static const Protocol shim6;
    static const Protocol wesp;
    static const Protocol rohc;
    // 143-252 		Unassigned protocol numbers
    static const Protocol gnp;    // INET specific: Generic Network Protocol; For testing in general // implemented
    static const Protocol test;   // For testing in general
    static const Protocol reserved;



};

} // namespace inet

#endif // ifndef __INET_PROTOCOL_H
