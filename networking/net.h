/* ============================================================================
 * Dionnex Kernel - Network Subsystem Core Header (networking/net.h)
 * ============================================================================
 */
#ifndef _DIONNEX_NETWORKING_NET_H
#define _DIONNEX_NETWORKING_NET_H

#include <kernel/kernel.h>

#define ETH_ALEN 6
#define ETH_HLEN 14
#define ETH_P_IP 0x0800

struct eth_hdr {
    uint8_t  dst_mac[ETH_ALEN];
    uint8_t  src_mac[ETH_ALEN];
    uint16_t ethertype;
} __attribute__((packed));

struct ip_hdr {
    uint8_t  version_ihl;
    uint8_t  tos;
    uint16_t tot_len;
    uint16_t id;
    uint16_t frag_off;
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t check;
    uint32_t saddr;
    uint32_t daddr;
} __attribute__((packed));

struct net_device {
    char name[16];
    uint8_t mac_addr[ETH_ALEN];
    uint32_t ip_addr;
    uint32_t rx_packets;
    uint32_t tx_packets;
    int (*transmit)(struct net_device* dev, const uint8_t* buffer, size_t len);
};

void net_subsystem_init(void);
struct net_device* net_register_device(const char* name, const uint8_t* mac);
void net_receive_packet(struct net_device* dev, const uint8_t* data, size_t len);

#endif /* _DIONNEX_NETWORKING_NET_H */
