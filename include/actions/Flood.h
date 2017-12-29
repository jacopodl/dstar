#ifndef DSTAR_FLOOD_H
#define DSTAR_FLOOD_H

#include <thread>

#include <DhcpAction.h>

class Flood : public virtual DhcpAction {
public:
    void action(Core *core) override;
};

#endif //DSTAR_FLOOD_H
