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

#include <iostream>
#include <cstring>
#include <thread>
#include <zconf.h>

#include "Core.h"

void Core::executeActions() {
    while (!this->stop)
        for (auto action:this->actions)
            action->action(this);
}

void Core::recvDhcp() {
    EthHeader *eth;
    Ipv4Header *ip;
    UdpHeader *udp;
    DhcpPacket *dhcp;
    PacketInfo pktInfo{};
    int size;
    unsigned short checksum;

    while (!this->stop) {
        if ((size = spark_read(this->sock, this->buf, nullptr)) < 0) {
            std::cerr << spark_strerror(size) << std::endl;
            exit(-1);
        }

        // ETHERNET
        eth = (EthHeader *) this->buf;
        if (ntohs(eth->eth_type) != ETHTYPE_IP)
            continue;

        memcpy(pktInfo.phisAddr.mac, eth->shwaddr, ETHHWASIZE);

        // IP
        ip = (Ipv4Header *) eth->data;
        if (ntohs(ip->len) - ((unsigned short) (ip->ihl * 4)) == 0) // Ignore empty packet
            continue;
        if (ip->protocol != 0x11) // UDP
            continue;
        checksum = ip->checksum;
        if (ip_checksum(ip) != checksum) // Sanity check
            continue;
        ip->checksum = checksum;

        pktInfo.ipSrc.ip = ip->saddr;
        pktInfo.ipDst.ip = ip->daddr;

        // UDP
        udp = (UdpHeader *) (((unsigned char *) ip) + (ip->ihl * 4));
        if (ntohs(udp->srcport) != 67 && ntohs(udp->srcport) != 68)
            continue;
        checksum = udp->checksum;
        if (udp_checksum(udp, ip) != checksum) // Sanity check
            continue;

        // DHCP
        dhcp = (DhcpPacket *) udp->data;
        if (dhcp->option != htonl(DHCP_MAGIC_COOKIE))
            continue;

        pktInfo.toServer = ntohs(udp->srcport) == 68 && ntohs(udp->dstport) == 67;

        for (auto action:this->actions)
            action->recvDhcpMsg(this, &pktInfo, dhcp);
    }
}

int Core::sendDhcpMsg(DhcpPacket *message, unsigned short len, PacketInfo *pktInfo) {
    unsigned char buf[SOCK_BUFSIZE];
    EthHeader *eth;
    Ipv4Header *ip;
    UdpHeader *udp;
    unsigned short src = 67;
    unsigned short dst = 68;

    if (len > DHCPPKTSIZE)
        return -EOVERFLOW;

    if (pktInfo->toServer) {
        src = 68;
        dst = 67;
    }

    eth = eth_inject_header(buf, &this->sock->iaddr, &pktInfo->phisAddr, ETHTYPE_IP);
    ip = ip_inject_header((unsigned char *) eth->data,
                          &pktInfo->ipSrc,
                          &pktInfo->ipDst, IPDEFIHL,
                          ip_mkid(),
                          (unsigned short) UDPHDRSIZE + len,
                          IPDEFTTL,
                          0x11);
    udp = udp_inject_header((unsigned char *) ip->data, src, dst, len);
    memcpy(udp->data, message, len);
    return spark_write(this->sock, buf, ETHHDRSIZE + IPHDRSIZE + UDPHDRSIZE + len);
}

void Core::openSocket(const std::string &interface) {
    int err;

    if ((err = spark_opensock((char *) interface.c_str(), SOCK_BUFSIZE, &this->sock)) != SPKERR_SUCCESS) {
        std::cerr << spark_strerror(err) << std::endl;
        exit(-1);
    }

    this->buf = new unsigned char[SOCK_BUFSIZE];

    this->thActions = std::thread(&Core::executeActions, this);

    this->recvDhcp();

    this->thActions.join();

    if (this->releaseOnExit) {
        std::cout << "Releasing addresses...\n";
        this->releaseSlots();
        std::cout << "All done!\n";
    }

    delete[] this->buf;
}

void Core::registerAction(DhcpAction *action) {
    this->actions.push_front(action);
}

void Core::addToFreeSlot(DhcpSlot *slot) {
    this->fsMutex.lock();
    this->freeSlots.push_front(slot);
    this->fsMutex.unlock();
}

void Core::releaseSlot(DhcpSlot *slot) {
    DhcpPacket packet{};
    PacketInfo pktInfo{};
    int err;

    pktInfo.phisAddr = slot->serverMac;
    pktInfo.ipSrc = slot->clientIp;
    pktInfo.ipDst = slot->serverIp;
    pktInfo.toServer = true;

    dhcp_inject_release((unsigned char *) &packet, &slot->clientMac, &slot->clientIp, &slot->serverIp, 0);

    if ((err = this->sendDhcpMsg(&packet, DHCPPKTSIZE, &pktInfo)) < 0)
        std::cerr << "Core(Release): " << spark_strerror(err) << std::endl;
}

void Core::releaseSlots() {
    this->fsMutex.lock();
    this->asMutex.lock();
    auto fsCursor = this->freeSlots.begin();
    auto asCursor = this->assignedSlots.begin();

    for (fsCursor; fsCursor != this->freeSlots.end(); fsCursor++) {
        this->releaseSlot(*fsCursor);
        delete (*fsCursor);
        usleep(5000);
    }

    for (asCursor; asCursor != this->assignedSlots.end(); asCursor++) {
        this->releaseSlot(*asCursor);
        delete (*asCursor);
        usleep(5000);
    }

    this->freeSlots.clear();
    this->assignedSlots.clear();
    this->fsMutex.unlock();
    this->asMutex.unlock();
}