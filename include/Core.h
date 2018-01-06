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

#ifndef DSTAR_CORE_H
#define DSTAR_CORE_H

#include <thread>
#include <list>
#include <string>
#include <spark.h>

#include <Options.h>
#include <PacketInfo.h>
#include <DhcpAction.h>
#include <DhcpSocket.h>
#include <DhcpPool.h>
#include <DhcpSlot.h>

class Core {
private:
    DhcpAction *action;
    std::thread thActions;

    void addDhcpDefaultOpt(DhcpPacket *message, DhcpSlot *slot, unsigned char type);

    void dhcpServer(DhcpPacket *message);

    void executeActions();

    void recvDhcp();

public:
    DhcpSocket socket;
    DhcpPool pool;
    Options options;

    void openSocket();

    void registerAction(DhcpAction *action);

    void releasePool();
};


#endif //DSTAR_CORE_H
