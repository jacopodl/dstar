#ifndef DSTAR_STARVATION_H
#define DSTAR_STARVATION_H

#include <DhcpAction.h>

class Starvation : public virtual DhcpAction {
private:
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    unsigned int lastXid = 0;
public:
    Starvation();

    ~Starvation();

    void action(Core *core) override;

    void recvDhcpMsg(Core *core, PacketInfo *pktInfo, DhcpPacket *dhcp) override;
};


#endif //DSTAR_STARVATION_H
