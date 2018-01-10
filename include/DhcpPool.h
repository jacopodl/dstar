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

#ifndef DSTAR_DHCPPOOL_H
#define DSTAR_DHCPPOOL_H

#include <list>
#include <mutex>
#include <spark.h>

#include <DhcpSlot.h>

class DhcpPool {
private:
    std::list<DhcpSlot *> slots;
    std::mutex mutex;
public:
    bool empty();

    DhcpSlot *popAndErase();

    DhcpSlot *getFreeSlot(netaddr_mac *clientMac, unsigned int newXid);

    DhcpSlot *getSlot(netaddr_mac *clientMac, unsigned int xid);

    void addSlot(DhcpSlot *slot);

    void releaseSlot(netaddr_ip *ip);
};


#endif //DSTAR_DHCPPOOL_H
