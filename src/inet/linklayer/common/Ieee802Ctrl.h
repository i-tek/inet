//
// Copyright (C) 2011 Andras Varga
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

#ifndef __INET_IEEE802CTRL_H
#define __INET_IEEE802CTRL_H

#include "inet/common/Protocol.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/IPacketControlInfo.h"
#include "inet/common/ISocketControlInfo.h"
#include "inet/common/IProtocolControlInfo.h"
#include "inet/common/IInterfaceControlInfo.h"
#include "inet/linklayer/contract/IMACProtocolControlInfo.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h"

namespace inet {

/**
 * Represents a IEEE 802 control info. More info in the Ieee802Ctrl.msg file
 * (and the documentation generated from it).
 */
class INET_API Ieee802Ctrl : public Ieee802Ctrl_Base, public IMACProtocolControlInfo, public IPacketControlInfo, public IProtocolControlInfo, public ISocketControlInfo, public IInterfaceControlInfo
{
  public:
    Ieee802Ctrl() : Ieee802Ctrl_Base() {}
    Ieee802Ctrl(const Ieee802Ctrl& other) : Ieee802Ctrl_Base(other) {}
    Ieee802Ctrl& operator=(const Ieee802Ctrl& other) { Ieee802Ctrl_Base::operator=(other); return *this; }

    virtual Ieee802Ctrl *dup() const override { return new Ieee802Ctrl(*this); }

    virtual int getControlInfoProtocolId() const override { return -1; }
    virtual int getPacketProtocolId() const override { return ProtocolGroup::ethertype.getProtocol(getNetworkProtocol())->getId(); }

    virtual MACAddress getSourceAddress() const override { return getSrc(); }
    virtual void setSourceAddress(const MACAddress& address) override { setSrc(address); }
    virtual MACAddress getDestinationAddress() const override { return getDest(); }
    virtual void setDestinationAddress(const MACAddress& address) override { setDest(address); };
    virtual int getNetworkProtocol() const override { return getEtherType(); }
    virtual void setNetworkProtocol(int protocolId) override { setEtherType(protocolId); }
    virtual int getInterfaceId() const override { return Ieee802Ctrl_Base::getInterfaceId(); }
    virtual void setInterfaceId(int interfaceId) override { Ieee802Ctrl_Base::setInterfaceId(interfaceId); }
    virtual int getSocketId() const override { return Ieee802Ctrl_Base::getSocketId(); }
    virtual void setSocketId(int socketId) override { Ieee802Ctrl_Base::setSocketId(socketId); }
};

} // namespace inet

#endif // ifndef __INET_IEEE802CTRL_H

