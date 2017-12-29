/*
	* dstar.
	* Copyright (C) 2018 Jacopo De Luca
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

#include <string>
#include <spark.h>

#include <PacketInfo.h>

#define SOCK_BUFSIZE    2048

class Core {
private:
    SpkSock *sock;
    unsigned char *buf;
    bool stop = false;

    void recvDhcp();

public:
    void openSocket(const std::string &interface);

    int sendDhcpMsg(DhcpPacket *message, unsigned short len, PacketInfo *pktInfo);

    void registerCallback();
};


#endif //DSTAR_CORE_H
