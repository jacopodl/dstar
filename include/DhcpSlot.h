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

#ifndef DSTAR_DHCPSLOT_H
#define DSTAR_DHCPSLOT_H

struct DhcpSlot {
    netaddr_ip clientIp;
    netaddr_mac clientMac;
    netaddr_mac fakeClientMac;
    netaddr_ip serverIp;
    netaddr_mac serverMac;
    netaddr_ip primaryDns;
    timeval timeStamp;
    unsigned int lease;
    unsigned int xid = 0;
    bool assigned = false;
};

#endif //DSTAR_DHCPSLOT_H
