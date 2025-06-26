#include "utils/ip_calculator.h"
#include "display/color_output.h"
#include <stdio.h>
#include <winsock2.h>

void calculate_subnet(const char *ip_address, int subnet_mask) {
    struct in_addr addr;
    if (inet_addr(ip_address) == INADDR_NONE) {
        printf("Invalid IP address\n");
        return;
    }
    addr.s_addr = inet_addr(ip_address);

    unsigned long ip = ntohl(addr.s_addr);
    unsigned long mask = (0xFFFFFFFF << (32 - subnet_mask)) & 0xFFFFFFFF;
    unsigned long network = ip & mask;
    unsigned long broadcast = network | ~mask;
    unsigned long first_host = network + 1;
    unsigned long last_host = broadcast - 1;
    unsigned long num_hosts = (1UL << (32 - subnet_mask)) - 2;


    struct in_addr network_addr, broadcast_addr, first_host_addr, last_host_addr, mask_addr;
    network_addr.s_addr = htonl(network);
    broadcast_addr.s_addr = htonl(broadcast);
    first_host_addr.s_addr = htonl(first_host);
    last_host_addr.s_addr = htonl(last_host);
    mask_addr.s_addr = htonl(mask);

    print_color(14, "    Subnet Calculation:\n"); // Yellow
    printf("      Network: %s\n", inet_ntoa(network_addr));
    printf("      Subnet Mask: %s\n", inet_ntoa(mask_addr));
    printf("      Broadcast: %s\n", inet_ntoa(broadcast_addr));
    printf("      First Host: %s\n", inet_ntoa(first_host_addr));
    printf("      Last Host: %s\n", inet_ntoa(last_host_addr));
    printf("      Number of Hosts: %lu\n", num_hosts);
}