//
// Copyright (C) 2012 Opensim Ltd
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

#ifndef __INET_IPROTOCOLREGISTRATIONLISTENER_H
#define __INET_IPROTOCOLREGISTRATIONLISTENER_H

#include "inet/common/Protocol.h"

namespace inet {

INET_API void registerProtocol(const Protocol& protocol, cGate *gate);

class INET_API IProtocolRegistrationListener
{
  public:
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate) = 0;
};

} // namespace inet

#endif // ifndef __INET_IPROTOCOLREGISTRATIONLISTENER_H

