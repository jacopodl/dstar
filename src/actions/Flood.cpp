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

#include <actions/Flood.h>
#include <iostream>

void Flood::action(DhcpSocket *socket) {
    DhcpPacket dhcpPacket{};
    PacketInfo pktInfo{};
    netaddr_mac(req);
    int err;

    eth_rndaddr(&req);
    eth_bcast(&pktInfo.phisAddr);
    pktInfo.ipDst.ip = 0xFFFFFFFF;
    pktInfo.toServer = true;

    dhcp_inject_discovery((unsigned char *) &dhcpPacket, &req, nullptr, DHCP_FLAGS_BROADCAST);
    if ((err = socket->sendDhcpMsg(&dhcpPacket, DHCPPKTSIZE, &pktInfo)) < 0)
        std::cerr << "Flood action: " << spark_strerror(err) << std::endl;
}