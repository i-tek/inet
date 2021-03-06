//
// (C) 2013 Opensim Ltd.
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
// Author: Andras Varga (andras@omnetpp.org)
//

#include "inet/networklayer/base/NetworkProtocolBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/IInterfaceControlInfo.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"
#include "inet/networklayer/contract/L3SocketCommand_m.h"

namespace inet {

NetworkProtocolBase::NetworkProtocolBase() :
    interfaceTable(nullptr)
{
}

void NetworkProtocolBase::initialize(int stage)
{
    LayeredProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        registerProtocol(Protocol::gnp, gate("transportOut"));
        registerProtocol(Protocol::gnp, gate("queueOut"));
    }
}

void NetworkProtocolBase::handleRegisterProtocol(const Protocol& protocol, cGate *gate) {
    Enter_Method("handleRegisterProtocol");
    protocolMapping.addProtocolMapping(ProtocolGroup::ipprotocol.getProtocolNumber(&protocol), gate->getIndex());
}

void NetworkProtocolBase::sendUp(cMessage *message)
{
    if (cPacket *packet = dynamic_cast<cPacket *>(message)) {
        INetworkProtocolControlInfo *controlInfo = check_and_cast<INetworkProtocolControlInfo *>(packet->getControlInfo());

        int protocol = controlInfo->getTransportProtocol();
        auto lowerBound = protocolIdToSocketDescriptors.lower_bound(protocol);
        auto upperBound = protocolIdToSocketDescriptors.upper_bound(protocol);
        bool hasSocket = lowerBound != upperBound;
        for (auto it = lowerBound; it != upperBound; it++) {
            INetworkProtocolControlInfo *controlInfoCopy = check_and_cast<INetworkProtocolControlInfo *>(check_and_cast<cObject *>(controlInfo)->dup());
            controlInfoCopy->setSocketId(it->second->socketId);
            cPacket *packetCopy = packet->dup();
            packetCopy->setControlInfo(check_and_cast<cObject *>(controlInfoCopy));
            emit(packetSentToUpperSignal, packetCopy);
            send(packetCopy, "transportOut");
        }
        if (protocolMapping.findOutputGateForProtocol(protocol) >= 0) {
            emit(packetSentToUpperSignal, packet);
            send(packet, "transportOut");
        }
        else if (!hasSocket) {
            EV_ERROR << "Transport protocol ID=" << protocol << " not connected, discarding packet\n";
            //TODO send an ICMP error: protocol unreachable
            // IMACProtocolControlInfo *controlInfo = dynamic_cast<IMACProtocolControlInfo *>(PK(datagram)->getControlInfo());
            // int inputInterfaceId = controlInfo != nullptr ? controlInfo->getInterfaceId() : -1;
            // sendToIcmp(datagram, inputInterfaceId, ICMP_DESTINATION_UNREACHABLE, ICMP_DU_PROTOCOL_UNREACHABLE);
            delete packet;
        }
        else
            delete packet;
    }
    else
        send(message, "transportOut");
}

void NetworkProtocolBase::sendDown(cMessage *message, int interfaceId)
{
    if (message->isPacket())
        emit(packetSentToLowerSignal, message);
    if (interfaceId != -1) {
        cObject *ctrl = message->getControlInfo();
        check_and_cast<IInterfaceControlInfo *>(ctrl)->setInterfaceId(interfaceId);
        send(message, "queueOut");
    }
    else {
        for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
            InterfaceEntry *interfaceEntry = interfaceTable->getInterface(i);
            if (interfaceEntry && !interfaceEntry->isLoopback()) {
                cMessage *duplicate = message->dup();
                cObject *ctrl = message->getControlInfo()->dup();
                check_and_cast<IInterfaceControlInfo *>(ctrl)->setInterfaceId(interfaceEntry->getInterfaceId());
                duplicate->setControlInfo(ctrl);
                send(duplicate, "queueOut");
            }
        }
        delete message;
    }
}

bool NetworkProtocolBase::isUpperMessage(cMessage *message)
{
    return message->getArrivalGate()->isName("transportIn");
}

bool NetworkProtocolBase::isLowerMessage(cMessage *message)
{
    return message->getArrivalGate()->isName("queueIn");
}

void NetworkProtocolBase::handleUpperCommand(cMessage *msg)
{
    if (L3SocketBindCommand *command = dynamic_cast<L3SocketBindCommand *>(msg->getControlInfo())) {
        ASSERT(command->getControlInfoProtocolId() == Protocol::gnp.getId());
        SocketDescriptor *descriptor = new SocketDescriptor(command->getSocketId(), command->getProtocolId());
        socketIdToSocketDescriptor[command->getSocketId()] = descriptor;
        protocolIdToSocketDescriptors.insert(std::pair<int, SocketDescriptor *>(command->getProtocolId(), descriptor));
        delete msg;
    }
    else if (L3SocketCloseCommand *command = dynamic_cast<L3SocketCloseCommand *>(msg->getControlInfo())) {
        ASSERT(command->getControlInfoProtocolId() == Protocol::gnp.getId());
        auto it = socketIdToSocketDescriptor.find(command->getSocketId());
        if (it != socketIdToSocketDescriptor.end()) {
            int protocol = it->second->protocolId;
            auto lowerBound = protocolIdToSocketDescriptors.lower_bound(protocol);
            auto upperBound = protocolIdToSocketDescriptors.upper_bound(protocol);
            for (auto jt = lowerBound; jt != upperBound; jt++) {
                if (it->second == jt->second) {
                    protocolIdToSocketDescriptors.erase(jt);
                    break;
                }
            }
            delete it->second;
            socketIdToSocketDescriptor.erase(it);
        }
        delete msg;
    }
    else
        LayeredProtocolBase::handleUpperCommand(msg);
}



} // namespace inet

