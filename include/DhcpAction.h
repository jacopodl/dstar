#ifndef DSTAR_DHCPACTION_H
#define DSTAR_DHCPACTION_H

#include <Core.h>
#include <spark.h>

class Core;

class DhcpAction {
public:
    virtual ~DhcpAction() = default;

    virtual void action(Core *core) = 0;

    virtual void recvDhcpMsg(Core *core, PacketInfo *pktInfo, DhcpPacket *dhcp) {};
};

#endif //DSTAR_DHCPACTION_H
