#ifndef NET_UTILS_H
#define NET_UTILS_H
#include <stdlib.h>

void print_routing_table();
void print_network_statistics();
int get_global_ip(char *ip_buffer, size_t buffer_size);
void print_global_ip(int hide_sensitive);

#endif
