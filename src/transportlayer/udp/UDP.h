//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004-2011 Andras Varga
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


#ifndef __INET_UDP_H
#define __INET_UDP_H

#include <map>
#include <list>

#include "ILifecycle.h"
#include "UDPControlInfo.h"

namespace inet {

class IInterfaceTable;
class IPv4ControlInfo;
class IPv6ControlInfo;
class ICMP;
class ICMPv6;
class UDPPacket;
class InterfaceEntry;

const int UDP_HEADER_BYTES = 8;

const bool DEFAULT_MULTICAST_LOOP = true;

/**
 * Implements the UDP protocol: encapsulates/decapsulates user data into/from UDP.
 *
 * More info in the NED file.
 */
class INET_API UDP : public cSimpleModule, public ILifecycle
{
  public:

    struct MulticastMembership
    {
        Address multicastAddress;
        int interfaceId;  // -1 = all
        UDPSourceFilterMode filterMode;
        std::vector<Address> sourceList;

        bool isSourceAllowed(Address sourceAddr);
    };

    // For a given multicastAddress and interfaceId there is at most one membership record.
    // Records are ordered first by multicastAddress, then by interfaceId (-1 interfaceId is the last)
    typedef std::vector<MulticastMembership*> MulticastMembershipTable;

    struct SockDesc
    {
        SockDesc(int sockId, int appGateIndex);
        int sockId;
        int appGateIndex;
        bool isBound;
        bool onlyLocalPortIsSet;
        bool reuseAddr;
        Address localAddr;
        Address remoteAddr;
        int localPort;
        int remotePort;
        bool isBroadcast;
        int multicastOutputInterfaceId;
        bool multicastLoop;
        int ttl;
        unsigned char typeOfService;
        MulticastMembershipTable multicastMembershipTable;

        MulticastMembershipTable::iterator findFirstMulticastMembership(const Address &multicastAddress);
        MulticastMembership *findMulticastMembership(const Address &multicastAddress, int interfaceId);
        void addMulticastMembership(MulticastMembership *membership);
        void deleteMulticastMembership(MulticastMembership *membership);
    };

    typedef std::list<SockDesc *> SockDescList;   // might contain duplicated local addresses if their reuseAddr flag is set
    typedef std::map<int,SockDesc *> SocketsByIdMap;
    typedef std::map<int,SockDescList> SocketsByPortMap;

  protected:
    // sockets
    SocketsByIdMap socketsByIdMap;
    SocketsByPortMap socketsByPortMap;

    // other state vars
    ushort lastEphemeralPort;
    IInterfaceTable *ift;
    ICMP *icmp;
    ICMPv6 *icmpv6;

    // statistics
    int numSent;
    int numPassedUp;
    int numDroppedWrongPort;
    int numDroppedBadChecksum;

    bool isOperational;

    static simsignal_t rcvdPkSignal;
    static simsignal_t sentPkSignal;
    static simsignal_t passedUpPkSignal;
    static simsignal_t droppedPkWrongPortSignal;
    static simsignal_t droppedPkBadChecksumSignal;

  protected:
    // utility: show current statistics above the icon
    virtual void updateDisplayString();

    // socket handling
    virtual SockDesc *getSocketById(int sockId);
    virtual SockDesc *getOrCreateSocket(int sockId, int gateIndex);
    virtual SockDesc *createSocket(int sockId, int gateIndex, const Address& localAddr, int localPort);
    virtual void bind(int sockId, int gateIndex, const Address& localAddr, int localPort);
    virtual void connect(int sockId, int gateIndex, const Address& remoteAddr, int remotePort);
    virtual void close(int sockId);
    virtual void clearAllSockets();
    virtual void setTimeToLive(SockDesc *sd, int ttl);
    virtual void setTypeOfService(SockDesc *sd, int typeOfService);
    virtual void setBroadcast(SockDesc *sd, bool broadcast);
    virtual void setMulticastOutputInterface(SockDesc *sd, int interfaceId);
    virtual void setMulticastLoop(SockDesc *sd, bool loop);
    virtual void setReuseAddress(SockDesc *sd, bool reuseAddr);
    virtual void joinMulticastGroups(SockDesc *sd, const std::vector<Address>& multicastAddresses, const std::vector<int> interfaceIds);
    virtual void leaveMulticastGroups(SockDesc *sd, const std::vector<Address>& multicastAddresses);
    virtual void blockMulticastSources(SockDesc *sd, InterfaceEntry *ie, Address multicastAddress, const std::vector<Address> &sourceList);
    virtual void unblockMulticastSources(SockDesc *sd, InterfaceEntry *ie, Address multicastAddress, const std::vector<Address> &sourceList);
    virtual void joinMulticastSources(SockDesc *sd, InterfaceEntry *ie, Address multicastAddress, const std::vector<Address> &sourceList);
    virtual void leaveMulticastSources(SockDesc *sd, InterfaceEntry *ie, Address multicastAddress, const std::vector<Address> &sourceList);
    virtual void setMulticastSourceFilter(SockDesc *sd, InterfaceEntry *ie, Address multicastAddress, UDPSourceFilterMode filterMode, const std::vector<Address> &sourceList);

    virtual void addMulticastAddressToInterface(InterfaceEntry *ie, const Address& multicastAddr);

    // ephemeral port
    virtual ushort getEphemeralPort();

    virtual SockDesc *findSocketForUnicastPacket(const Address& localAddr, ushort localPort, const Address& remoteAddr, ushort remotePort);
    virtual std::vector<SockDesc*> findSocketsForMcastBcastPacket(const Address& localAddr, ushort localPort, const Address& remoteAddr, ushort remotePort, bool isMulticast, bool isBroadcast);
    virtual SockDesc *findFirstSocketByLocalAddress(const Address& localAddr, ushort localPort);
    virtual void sendUp(cPacket *payload, SockDesc *sd, const Address& srcAddr, ushort srcPort, const Address& destAddr, ushort destPort, int interfaceId, int ttl, unsigned char tos);
    virtual void sendDown(cPacket *appData, const Address& srcAddr, ushort srcPort, const Address& destAddr, ushort destPort, int interfaceId, bool multicastLoop, int ttl, unsigned char tos);
    virtual void processUndeliverablePacket(UDPPacket *udpPacket, cObject *ctrl);
    virtual void sendUpErrorIndication(SockDesc *sd, const Address& localAddr, ushort localPort, const Address& remoteAddr, ushort remotePort);

    // process an ICMP error packet
    virtual void processICMPError(cPacket *icmpErrorMsg); // TODO use ICMPMessage

    // process UDP packets coming from IP
    virtual void processUDPPacket(UDPPacket *udpPacket);

    // process packets from application
    virtual void processPacketFromApp(cPacket *appData);

    // process commands from application
    virtual void processCommandFromApp(cMessage *msg);

    // create a blank UDP packet; override to subclass UDPPacket
    virtual UDPPacket *createUDPPacket(const char *name);

    // ILifeCycle:
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);

  public:
    UDP();
    virtual ~UDP();

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg);
};

} //namespace


#endif
