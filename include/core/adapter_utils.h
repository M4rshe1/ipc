#ifndef ADAPTER_UTILS_H
#define ADAPTER_UTILS_H


#include <windows.h>
#include <iphlpapi.h>

unsigned long cidr_to_mask(int cidr);
unsigned long get_broadcast_address(unsigned long ip_address,
                                    unsigned long subnet_mask);
unsigned long get_network_id(unsigned long ip_address,
                             unsigned long subnet_mask);

#endif
