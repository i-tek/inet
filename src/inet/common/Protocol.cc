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

#include "inet/common/Protocol.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/networklayer/common/IPProtocolId_m.h"

namespace inet {

int Protocol::nextId = 0;
std::map<int, const Protocol *> Protocol::idToProtocol;
std::map<std::string, const Protocol *> Protocol::nameToProtocol;

Protocol::Protocol(const char *name) :
    id(nextId++),
    name(name)
{
    idToProtocol[id] = this;
    nameToProtocol[name] = this;
}

const Protocol *Protocol::findProtocol(int id)
{
    auto it = idToProtocol.find(id);
    return it != idToProtocol.end() ? it->second : nullptr;
}

const Protocol *Protocol::getProtocol(int id)
{
    const Protocol *protocol = findProtocol(id);
    if (protocol != nullptr)
        return protocol;
    else
        throw cRuntimeError("Unknown protocol: id = %d" , id);
}

const Protocol *Protocol::findProtocol(const char *name)
{
    auto it = nameToProtocol.find(name);
    return it != nameToProtocol.end() ? it->second : nullptr;
}

const Protocol *Protocol::getProtocol(const char *name)
{
    const Protocol *protocol = findProtocol(name);
    if (protocol != nullptr)
        return protocol;
    else
        throw cRuntimeError("Unknown protocol: name = %s" , name);
}

const Protocol Protocol::aodv("aodv"); // implemented
const Protocol Protocol::arp("arp"); // implemented
const Protocol Protocol::bgp("bgp"); // implemented
const Protocol Protocol::dsdv2("dsdv2"); // implemented
const Protocol Protocol::dymo("dymo");
const Protocol Protocol::ethernet("ethernet");
const Protocol Protocol::gpsr("gpsr");
const Protocol Protocol::ieee80211("ieee80211");


// ip protocols (in order of ProtocolGroup)
const Protocol Protocol::hopoptv6("hopoptv6");
const Protocol Protocol::icmpv4("icmpv4"); // implemented
const Protocol Protocol::igmp("igmp"); // implemented
const Protocol Protocol::ggp("ggp");
const Protocol Protocol::ipv4("ipv4"); // implemented
const Protocol Protocol::st("st");
const Protocol Protocol::tcp("tcp"); // implemented
const Protocol Protocol::cbt("cbt");
const Protocol Protocol::egp("egp"); // implemented
const Protocol Protocol::igp("igp"); // implemented
const Protocol Protocol::bbnrccmon("bbnrccmon");
const Protocol Protocol::nvp2("nvp2");
const Protocol Protocol::pup("pup");
const Protocol Protocol::argus("argus"); // deprecated
const Protocol Protocol::emcon("emcon");
const Protocol Protocol::xnet("xnet");
const Protocol Protocol::chaos("chaos");
const Protocol Protocol::udp("udp"); // implemented
const Protocol Protocol::mux("mux");
const Protocol Protocol::dcn("dcn");
const Protocol Protocol::hmp("hmp");
const Protocol Protocol::prm("prm");
const Protocol Protocol::xnsidp("xnsidp");
const Protocol Protocol::trunk1("trunk1");
const Protocol Protocol::trunk2("trunk2");
const Protocol Protocol::leaf1("leaf1");
const Protocol Protocol::leaf2("leaf2");
const Protocol Protocol::rdp("rdp");
const Protocol Protocol::irtp("irtp");
const Protocol Protocol::isotp4("isotp4");
const Protocol Protocol::netblt("netblt");
const Protocol Protocol::mfe("mfe");
const Protocol Protocol::merit("merit");
const Protocol Protocol::dccp("dccp");
const Protocol Protocol::tpcp("tpcp");
const Protocol Protocol::idpr("idpr");
const Protocol Protocol::xtp("xtp"); // implemented
const Protocol Protocol::ddp("ddp");
const Protocol Protocol::idprcmtp("idprcmtp");
const Protocol Protocol::tppp("tppp");
const Protocol Protocol::il("il");
const Protocol Protocol::ipv6("ipv6"); // implemented
const Protocol Protocol::sdrp("sdrp");
const Protocol Protocol::routev6("routev6");
const Protocol Protocol::fragv6("fragv6");
const Protocol Protocol::idrp("idrp");
const Protocol Protocol::rsvp("rsvp"); // implemented
const Protocol Protocol::gre("gre");
const Protocol Protocol::dsr("dsr"); // implemented
const Protocol Protocol::bna("bna");
const Protocol Protocol::esp("esp");
const Protocol Protocol::ah("ah");
const Protocol Protocol::inlsp("inlsp");
const Protocol Protocol::swipe("swipe"); // deprecated
const Protocol Protocol::narp("narp");
const Protocol Protocol::mobilev4("mobilev4");
const Protocol Protocol::tlsp("tlsp");
const Protocol Protocol::skip("skip");
const Protocol Protocol::icmpv6("icmpv6"); // implemented
const Protocol Protocol::nonxtv6("nonxtv6");
const Protocol Protocol::optsv6("optsv6");
const Protocol Protocol::ahint("ahint");  // Any host internal protocol
const Protocol Protocol::cftp("cftp");
const Protocol Protocol::alan("alan");  // Any local network
const Protocol Protocol::satexpak("satexpak");
const Protocol Protocol::kryptolan("kryptolan");
const Protocol Protocol::rvd("rvd");
const Protocol Protocol::ippc("ippc");
const Protocol Protocol::distfile("distfile"); // Any distributed file system
const Protocol Protocol::satmon("satmon");
const Protocol Protocol::visa("visa");
const Protocol Protocol::ipcv("ipcv");
const Protocol Protocol::cpnx("cpnx");
const Protocol Protocol::cphb("cphb");
const Protocol Protocol::wsn("wsn");
const Protocol Protocol::pvp("pvp");
const Protocol Protocol::brsatmon("brsatmon");
const Protocol Protocol::sunnd("sunnd");
const Protocol Protocol::wbmon("wbmon");
const Protocol Protocol::wbexpak("wbexpak");
const Protocol Protocol::isoip("isoip");
const Protocol Protocol::vmtp("vmtp");
const Protocol Protocol::svmtp("svmtp");
const Protocol Protocol::vines("vines");
const Protocol Protocol::ttp("ttp"); // Second definition: { 84, &Protocol::iptm },
const Protocol Protocol::nsfnet("nsfnet");
const Protocol Protocol::dgp("dgp");
const Protocol Protocol::tcf("tcf");
const Protocol Protocol::eigrp("eigrp");
const Protocol Protocol::ospf("ospf"); // implemented
const Protocol Protocol::sprite("sprite");
const Protocol Protocol::larp("larp");
const Protocol Protocol::mtp("mtp");
const Protocol Protocol::ax25("ax25");
const Protocol Protocol::ipip("ipip");
const Protocol Protocol::micp("micp"); // deprecated
const Protocol Protocol::sccsp("sccsp");
const Protocol Protocol::etherip("etherip");
const Protocol Protocol::encap("encap");
const Protocol Protocol::privenc("privenc"); // Custom private encryption scheme
const Protocol Protocol::gmtp("gmtp");
const Protocol Protocol::ifmp("ifmp");
const Protocol Protocol::pnni("pnni");
const Protocol Protocol::pim("pim"); // implemented
const Protocol Protocol::aris("aris");
const Protocol Protocol::scps("scps");
const Protocol Protocol::qnx("qnx");
const Protocol Protocol::an("an");
const Protocol Protocol::ipcomp("ipcomp");
const Protocol Protocol::snp("snp");
const Protocol Protocol::compaq("compaq");
const Protocol Protocol::ipxip("ipxip");
const Protocol Protocol::vrrp("vrrp");
const Protocol Protocol::pgm("pgm");
const Protocol Protocol::zerohop("zerohop"); // Custom 0-hop protocols
const Protocol Protocol::l2tp("l2tp");
const Protocol Protocol::ddx("ddx");
const Protocol Protocol::iatp("iatp");
const Protocol Protocol::stp("stp");
const Protocol Protocol::srp("srp");
const Protocol Protocol::uti("uti");
const Protocol Protocol::smp("smp");
const Protocol Protocol::sm("sm");   // deprecated
const Protocol Protocol::ptp("ptp");
const Protocol Protocol::isisv4("isisv4");
const Protocol Protocol::fire("fire");
const Protocol Protocol::crtp("crtp");
const Protocol Protocol::crudp("crudp");
const Protocol Protocol::sscopmce("sscopmce");
const Protocol Protocol::iplt("iplt");
const Protocol Protocol::sps("sps");
const Protocol Protocol::pipe("pipe");
const Protocol Protocol::sctp("sctp"); // implemented
const Protocol Protocol::fc("fc");
const Protocol Protocol::rsvpignore("rsvpignore");
const Protocol Protocol::mobilityv6("mobilityv6");
const Protocol Protocol::udplite("udplite");
const Protocol Protocol::mplsip("mplsip");
const Protocol Protocol::manet("manet"); // implemented
const Protocol Protocol::hip("hip");
const Protocol Protocol::shim6("shim6");
const Protocol Protocol::wesp("wesp");
const Protocol Protocol::rohc("rohc");
// 143-252 		Unassigned protocol numbers
const Protocol Protocol::gnp("gnp");    // INET specific: Generic Network Protocol; For testing in general // implemented
const Protocol Protocol::test("test");   // For testing in general
const Protocol Protocol::reserved("reserved");

} // namespace inet

