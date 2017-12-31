#ifndef DSTAR_DHCPSLOT_H
#define DSTAR_DHCPSLOT_H

struct DhcpSlot {
    netaddr_ip clientIp;
    netaddr_mac clientMac;
    netaddr_ip serverIp;
    netaddr_mac serverMac;
    timeval timeStamp;
    unsigned int lease;
    unsigned int xid = 0;
    bool assigned = false;
};

#endif //DSTAR_DHCPSLOT_H
