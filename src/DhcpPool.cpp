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

#include <sys/time.h>
#include <DhcpPool.h>

bool DhcpPool::empty() {
    return this->slots.empty();
}

DhcpSlot *DhcpPool::popAndErase() {
    DhcpSlot *slot = nullptr;
    this->mutex.lock();
    slot = this->slots.front();
    this->slots.pop_front();
    this->mutex.unlock();
    return slot;
}

DhcpSlot *DhcpPool::getFreeSlot() {
    DhcpSlot *slot = nullptr;
    timeval now{};

    gettimeofday(&now, nullptr);

    this->mutex.lock();
    for (auto cursor:this->slots) {
        if (cursor->xid == 0) {
            slot = cursor;
            break;
        }
        if (!cursor->assigned) {
            if ((now.tv_sec - cursor->timeStamp.tv_sec) >= 30) {
                cursor->xid = 0;
                slot = cursor;
                break;
            }
        }
        // if ((now.tv_sec - (cursor->timeStamp.tv_sec + cursor->lease)))
    }
    this->mutex.unlock();
    if (slot != nullptr)
        gettimeofday(&slot->timeStamp, nullptr);
    return slot;
}

DhcpSlot *DhcpPool::getSlotByXid(unsigned int xid) {
    this->mutex.lock();
    for (auto cursor:this->slots) {
        if (cursor->xid == xid) {
            this->mutex.unlock();
            return cursor;
        }
    }
    this->mutex.unlock();
    return nullptr;
}

void DhcpPool::addSlot(DhcpSlot *slot) {
    this->mutex.lock();
    this->slots.push_front(slot);
    this->mutex.unlock();
}

void DhcpPool::releaseSlot(netaddr_ip *ip) {
    this->mutex.lock();
    for (auto cursor:this->slots)
        if (ip_equals(&cursor->clientIp, ip) && cursor->assigned) {
            cursor->xid = 0;
            cursor->assigned = false;
            break;
        }
    this->mutex.unlock();
}