/*
	* dstar
	* Copyright (C) 2017 - 2018 Jacopo De Luca
	*
	* This program is free software: you can redistribute it and/or modify
	* it under the terms of the GNU General Public License as published by
	* the Free Software Foundation, either version 3 of the License, or
	* (at your option) any later version.
	* This program is distributed in the hope that it will be useful,
	* but WITHOUT ANY WARRANTY; without even the implied warranty of
	* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	* GNU General Public License for more details.
	* You should have received a copy of the GNU General Public License
	* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DSTAR_DHCPACTION_H
#define DSTAR_DHCPACTION_H

#include <spark.h>

#include <DhcpSocket.h>
#include <DhcpPool.h>

class DhcpAction {
protected:
    unsigned short waiting;
public:
    virtual ~DhcpAction() = default;

    unsigned short getWaitingTime() {
        return this->waiting;
    }

    virtual void action(DhcpSocket *socket) = 0;

    virtual void recvDhcpMsg(DhcpSocket *socket, DhcpPool *pool, PacketInfo *pktInfo, DhcpPacket *dhcp) {};
};

#endif //DSTAR_DHCPACTION_H
