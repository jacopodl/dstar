#ifndef DSTAR_OPTIONS_H
#define DSTAR_OPTIONS_H

#include <string>
#include <spark.h>

#define ATKMODE_FLOOD       (1<<0)
#define ATKMODE_RELEASE     (1<<1)
#define ATKMODE_STARVATION  (1<<2)

struct Options {
    std::string iface;
    int mode = 0;

    bool stop = false;
    bool releaseOnExit = true;
    bool enableServer = false;

    // DhcpServerOptions
    unsigned short lease = 0;
    netaddr_ip gateway{};
    netaddr_ip primaryDns{};
};

#endif //DSTAR_OPTIONS_H