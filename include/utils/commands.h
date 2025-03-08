#ifndef IPCONFIG_COMMANDS_H
#define IPCONFIG_COMMANDS_H

void renew_dhcp_lease(const char *adapter_name);
void release_dhcp_lease(const char *adapter_name);
void nslookup(const char *hostname);
void print_routing_table();
void print_network_statistics();

#endif
