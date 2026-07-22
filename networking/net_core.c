/* ============================================================================
 * Dionnex Kernel - Network Subsystem Core Implementation (networking/net_core.c)
 * ============================================================================
 */
#include <networking/net.h>

static struct net_device loopback_dev;
static int net_initialized = 0;

int net_loopback_transmit(struct net_device* dev, const uint8_t* buffer, size_t len) {
    if (!dev || !buffer || len == 0) return -1;
    dev->tx_packets++;
    dev->rx_packets++;
    return (int)len;
}

void net_subsystem_init(void) {
    if (net_initialized) return;

    memset(&loopback_dev, 0, sizeof(struct net_device));
    strcpy(loopback_dev.name, "lo");
    loopback_dev.ip_addr = 0x7F000001; // 127.0.0.1
    loopback_dev.transmit = net_loopback_transmit;

    net_initialized = 1;
    vga_puts("[NET] Subsistema de Red IPv4 / Ethernet inicializado (lo: 127.0.0.1).\n");
}

struct net_device* net_register_device(const char* name, const uint8_t* mac) {
    struct net_device* dev = (struct net_device*)kmalloc(sizeof(struct net_device));
    if (!dev) return NULL;

    memset(dev, 0, sizeof(struct net_device));
    strncpy(dev->name, name, 15);
    if (mac) {
        memcpy(dev->mac_addr, mac, ETH_ALEN);
    }
    return dev;
}

void net_receive_packet(struct net_device* dev, const uint8_t* data, size_t len) {
    if (!dev || !data || len < sizeof(struct eth_hdr)) return;
    dev->rx_packets++;
}
