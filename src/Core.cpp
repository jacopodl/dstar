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

        if (this->enableServer)
            this->dhcpServer(&packet);
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
                        DHCP_FLAGS_BROADCAST,
                        0,
                        nullptr,
                        &slot->clientIp,
                        &this->socket.netinfo.ipAddr,
                        nullptr,
                        nullptr,
                        nullptr);
        memcpy(response.chaddr, message->chaddr, ETHHWASIZE);
        response.options[tmp++] = DHCP_MESSAGE_TYPE;
        response.options[tmp++] = 0x01;
        response.options[tmp++] = DHCP_OFFER;
        response.options[tmp] = 0xFF;

        dhcp_append_option(&response, DHCP_SERVER_IDENTIFIER, IPADDRSIZE,
                           (unsigned char *) &this->socket.netinfo.ipAddr.ip);
        dhcp_append_option(&response, DHCP_REQ_ROUTERS, IPADDRSIZE, (unsigned char *) &this->socket.netinfo.ipAddr.ip);
        dhcp_append_option(&response, DHCP_REQ_SUBMASK, IPADDRSIZE, (unsigned char *) &this->socket.netinfo.netMask.ip);
        tmp = htonl(slot->lease);
        std::cout << dhcp_append_option(&response, DHCP_ADDR_LEASE_TIME, 4, (unsigned char *) &tmp);
        dhcp_append_option(&response, DHCP_REQ_DNS, IPADDRSIZE, (unsigned char *) &this->socket.netinfo.ipAddr.ip);

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

        slot->assigned = true;
        dhcp_inject_raw((unsigned char *) &response,
                        DHCP_OP_BOOT_REPLY,
                        0,
                        message->xid,
                        DHCP_FLAGS_BROADCAST,
                        0,
                        nullptr,
                        &slot->clientIp,
                        &this->socket.netinfo.ipAddr,
                        nullptr,
                        nullptr,
                        nullptr);
        memcpy(response.chaddr, message->chaddr, ETHHWASIZE);
        response.options[tmp++] = DHCP_MESSAGE_TYPE;
        response.options[tmp++] = 0x01;
        response.options[tmp++] = DHCP_ACK;
        response.options[tmp] = 0xFF;

        dhcp_append_option(&response, DHCP_SERVER_IDENTIFIER, IPADDRSIZE,
                           (unsigned char *) &this->socket.netinfo.ipAddr.ip);
        dhcp_append_option(&response, DHCP_REQ_ROUTERS, IPADDRSIZE, (unsigned char *) &this->socket.netinfo.ipAddr.ip);
        dhcp_append_option(&response, DHCP_REQ_SUBMASK, IPADDRSIZE, (unsigned char *) &this->socket.netinfo.netMask.ip);
        tmp = htonl(slot->lease);
        std::cout << dhcp_append_option(&response, DHCP_ADDR_LEASE_TIME, 4, (unsigned char *) &tmp);
        dhcp_append_option(&response, DHCP_REQ_DNS, IPADDRSIZE, (unsigned char *) &this->socket.netinfo.ipAddr.ip);

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