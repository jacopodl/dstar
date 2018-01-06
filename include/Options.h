#ifndef DSTAR_OPTIONS_H
#define DSTAR_OPTIONS_H

#include <string>
#include <spark.h>

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