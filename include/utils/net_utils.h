#ifndef NET_UTILS_H
#define NET_UTILS_H
#include <stdlib.h>


int get_global_ip(char *ip_buffer, size_t buffer_size);
void print_global_ip(int hide_sensitive);
int wake_on_lan(const char *target);
int resolve_mac_address(const char *ip_address, char *mac_address_buffer,
                        int buffer_size);
int resolve_ip_address(const char *mac_address, char *ip_address_buffer,
                       int buffer_size);

#endif
