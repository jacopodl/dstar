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

#ifndef DSTAR_STARVATION_H
#define DSTAR_STARVATION_H

#include <DhcpAction.h>

class Starvation : public virtual DhcpAction {
private:
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    unsigned int lastXid = 0;
public:
    Starvation();

    ~Starvation();

    void action(DhcpSocket *socket) override;

    void recvDhcpMsg(DhcpSocket *socket, DhcpPool *pool, PacketInfo *pktInfo, DhcpPacket *dhcp) override;
};


#endif //DSTAR_STARVATION_H
