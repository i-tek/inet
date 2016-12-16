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

#include "inet/common/ProtocolGroup.h"

namespace inet {

ProtocolGroup::ProtocolGroup(const char *name, std::map<int, const Protocol *> protocolNumberToProtocol) :
    name(name),
    protocolNumberToProtocol(protocolNumberToProtocol)
{
    for (auto it : protocolNumberToProtocol)
        protocolToProtocolNumber[it.second] = it.first;
}

const Protocol *ProtocolGroup::findProtocol(int protocolNumber) const
{
    auto it = protocolNumberToProtocol.find(protocolNumber);
    return it != protocolNumberToProtocol.end() ? it->second : nullptr;
}

const Protocol *ProtocolGroup::getProtocol(int protocolNumber) const
{
    auto protocol = findProtocol(protocolNumber);
    if (protocol != nullptr)
        return protocol;
    else
        throw cRuntimeError("Unknown protocol: number = %d", protocolNumber);
}

int ProtocolGroup::findProtocolNumber(const Protocol *protocol) const
{
    auto it = protocolToProtocolNumber.find(protocol);
    return it != protocolToProtocolNumber.end() ? it->second : -1;
}

int ProtocolGroup::getProtocolNumber(const Protocol *protocol) const
{
    auto protocolNumber = findProtocolNumber(protocol);
    if (protocolNumber != -1)
        return protocolNumber;
    else
        throw cRuntimeError("Unknown protocol: id = %d, name = %s", protocol->getId(), protocol->getName());
}


//FIXME use constants instead of numbers

// excerpt from http://www.iana.org/assignments/ieee-802-numbers/ieee-802-numbers.xhtml
const ProtocolGroup ProtocolGroup::ethertype("ethertype", {
    { 0x0800, &Protocol::ipv4 },
    { 0x0806, &Protocol::arp},
    { 0x86DD, &Protocol::ipv6 },
    { 0x86FF, &Protocol::gnp },         // ETHERTYPE_INET_GENERIC
});

// excerpt from http://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml
const ProtocolGroup ProtocolGroup::ipprotocol("ipprotocol", {

    { 0, &Protocol::hopoptv6 },
    { 1, &Protocol::icmpv4 }, // implemented
    { 2, &Protocol::igmp }, // implemented
    { 3, &Protocol::ggp },
    { 4, &Protocol::ipv4 }, // implemented
    { 5, &Protocol::st },
    { 6, &Protocol::tcp }, // implemented
    { 7, &Protocol::cbt },
    { 8, &Protocol::egp }, // implemented
    { 9, &Protocol::igp }, // implemented
    { 10, &Protocol::bbnrccmon },
    { 11, &Protocol::nvp2 },
    { 12, &Protocol::pup },
    { 13, &Protocol::argus }, // deprecated
    { 14, &Protocol::emcon },
    { 15, &Protocol::xnet },
    { 16, &Protocol::chaos },
    { 17, &Protocol::udp }, // implemented
    { 18, &Protocol::mux },
    { 19, &Protocol::dcn },
    { 20, &Protocol::hmp },
    { 21, &Protocol::prm },
    { 22, &Protocol::xnsidp },
    { 23, &Protocol::trunk1 },
    { 24, &Protocol::trunk2 },
    { 25, &Protocol::leaf1 },
    { 26, &Protocol::leaf2 },
    { 27, &Protocol::rdp },
    { 28, &Protocol::irtp },
    { 29, &Protocol::isotp4 },
    { 30, &Protocol::netblt },
    { 31, &Protocol::mfe },
    { 32, &Protocol::merit },
    { 33, &Protocol::dccp },
    { 34, &Protocol::tpcp },
    { 35, &Protocol::idpr },
    { 36, &Protocol::xtp }, // implemented
    { 37, &Protocol::ddp },
    { 38, &Protocol::idprcmtp },
    { 39, &Protocol::tppp },
    { 40, &Protocol::il },
    { 41, &Protocol::ipv6 }, // implemented
    { 42, &Protocol::sdrp },
    { 43, &Protocol::routev6 },
    { 44, &Protocol::fragv6 },
    { 45, &Protocol::idrp },
    { 46, &Protocol::rsvp }, // implemented
    { 47, &Protocol::gre },
    { 48, &Protocol::dsr }, // implemented
    { 49, &Protocol::bna },
    { 50, &Protocol::esp },
    { 51, &Protocol::ah },
    { 52, &Protocol::inlsp },
    { 53, &Protocol::swipe }, // deprecated
    { 54, &Protocol::narp },
    { 55, &Protocol::mobilev4 },
    { 56, &Protocol::tlsp },
    { 57, &Protocol::skip },
    { 58, &Protocol::icmpv6 }, // implemented
    { 59, &Protocol::nonxtv6 },
    { 60, &Protocol::optsv6 },
    { 61, &Protocol::ahint },  // Any host internal protocol
    { 62, &Protocol::cftp },
    { 63, &Protocol::alan },  // Any local network
    { 64, &Protocol::satexpak },
    { 65, &Protocol::kryptolan },
    { 66, &Protocol::rvd },
    { 67, &Protocol::ippc },
    { 68, &Protocol::distfile }, // Any distributed file system
    { 69, &Protocol::satmon },
    { 70, &Protocol::visa },
    { 71, &Protocol::ipcv },
    { 72, &Protocol::cpnx },
    { 73, &Protocol::cphb },
    { 74, &Protocol::wsn },
    { 75, &Protocol::pvp },
    { 76, &Protocol::brsatmon },
    { 77, &Protocol::sunnd },
    { 78, &Protocol::wbmon },
    { 79, &Protocol::wbexpak },
    { 80, &Protocol::isoip },
    { 81, &Protocol::vmtp },
    { 82, &Protocol::svmtp },
    { 83, &Protocol::vines },
    { 84, &Protocol::ttp }, // Second definition: { 84, &Protocol::iptm },
    { 85, &Protocol::nsfnet },
    { 86, &Protocol::dgp },
    { 87, &Protocol::tcf },
    { 88, &Protocol::eigrp },
    { 89, &Protocol::ospf }, // implemented
    { 90, &Protocol::sprite },
    { 91, &Protocol::larp },
    { 92, &Protocol::mtp },
    { 93, &Protocol::ax25 },
    { 94, &Protocol::ipip },
    { 95, &Protocol::micp }, // deprecated
    { 96, &Protocol::sccsp },
    { 97, &Protocol::etherip },
    { 98, &Protocol::encap },
    { 99, &Protocol::privenc }, // Custom private encryption scheme
    { 100, &Protocol::gmtp },
    { 101, &Protocol::ifmp },
    { 102, &Protocol::pnni },
    { 103, &Protocol::pim }, // implemented
    { 104, &Protocol::aris },
    { 105, &Protocol::scps },
    { 106, &Protocol::qnx },
    { 107, &Protocol::an },
    { 108, &Protocol::ipcomp },
    { 109, &Protocol::snp },
    { 110, &Protocol::compaq },
    { 111, &Protocol::ipxip },
    { 112, &Protocol::vrrp },
    { 113, &Protocol::pgm },
    { 114, &Protocol::zerohop }, // Custom 0-hop protocols
    { 115, &Protocol::l2tp },
    { 116, &Protocol::ddx },
    { 117, &Protocol::iatp },
    { 118, &Protocol::stp },
    { 119, &Protocol::srp },
    { 120, &Protocol::uti },
    { 121, &Protocol::smp },
    { 122, &Protocol::sm },   // deprecated
    { 123, &Protocol::ptp },
    { 124, &Protocol::isisv4 },
    { 125, &Protocol::fire },
    { 126, &Protocol::crtp },
    { 127, &Protocol::crudp },
    { 128, &Protocol::sscopmce },
    { 129, &Protocol::iplt },
    { 130, &Protocol::sps },
    { 131, &Protocol::pipe },
    { 132, &Protocol::sctp }, // implemented
    { 133, &Protocol::fc },
    { 134, &Protocol::rsvpignore },
    { 135, &Protocol::mobilityv6 },
    { 136, &Protocol::udplite },
    { 137, &Protocol::mplsip },
    { 138, &Protocol::manet }, // implemented
    { 139, &Protocol::hip },
    { 140, &Protocol::shim6 },
    { 141, &Protocol::wesp },
    { 142, &Protocol::rohc },
    // 143-252 		Unassigned protocol numbers
    { 253, &Protocol::gnp },    // INET specific: Generic Network Protocol; For testing in general  // implemented
    { 254, &Protocol::test },   // For testing in general
    { 255, &Protocol::reserved }

});

} // namespace inet

