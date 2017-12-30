#ifndef DSTAR_DHCPSLOT_H
#define DSTAR_DHCPSLOT_H

struct DhcpSlot {
    netaddr_ip clientIp;
    netaddr_mac clientMac;
    netaddr_ip serverIp;
    netaddr_mac serverMac;
    int lease;
    timeval timeStamp;
};

#endif //DSTAR_DHCPSLOT_H
