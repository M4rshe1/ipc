#include "utils/commands.h"
#include <stdio.h>
#include <stdlib.h>

void renew_dhcp_lease(const char *adapter_name) {
    char command[256];
    snprintf(command, sizeof(command), "ipconfig /renew \"%s\"", adapter_name);
    printf("Renewing DHCP lease for adapter '%s'...\n", adapter_name);
    system(command);
}

void release_dhcp_lease(const char *adapter_name) {
    char command[256];
    snprintf(command, sizeof(command), "ipconfig /release \"%s\"", adapter_name);
    printf("Releasing DHCP lease for adapter '%s'...\n", adapter_name);
    system(command);
}

void nslookup(const char *hostname) {
    char command[256];
    snprintf(command, sizeof(command), "nslookup %s", hostname);
    system(command);
}

void print_routing_table() { system("route print"); }

void print_network_statistics() { system("netstat -e"); }