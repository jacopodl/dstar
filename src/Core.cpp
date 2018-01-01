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

#include <iostream>
#include <cstring>
#include <zconf.h>

#include "Core.h"

void Core::executeActions() {
    while (!this->stop)
        for (auto action:this->actions)
            action->action(&this->socket);
}

void Core::recvDhcp() {
    DhcpPacket packet{};
    PacketInfo pktInfo{};

    while (!this->stop) {
        if (this->socket.recvDhcp((unsigned char *) &packet, &pktInfo) == 0)
            continue;

        for (auto action:this->actions)
            action->recvDhcpMsg(&this->socket, &this->pool, &pktInfo, &packet);
    }
}

void Core::openSocket(const std::string &interface) {
    this->socket.openSocket(interface);

    this->thActions = std::thread(&Core::executeActions, this);

    this->recvDhcp();

    this->thActions.join();

    if (this->releaseOnExit) {
        std::cout << "Releasing addresses...\n";
        this->releasePool();
        std::cout << "All done!\n";
    }
}

void Core::registerAction(DhcpAction *action) {
    this->actions.push_front(action);
}

void Core::releasePool() {
    DhcpSlot *slot;
    while (!this->pool.empty()) {
        slot = this->pool.popAndErase();
        this->socket.sendDhcpRelease(slot);
        delete slot;
        usleep(5000);
    }
}

