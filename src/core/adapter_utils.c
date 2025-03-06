#include "core/adapter_utils.h"

unsigned long cidr_to_mask(int cidr) {
    return cidr ? 0xFFFFFFFF << (32 - cidr) : 0;
}

unsigned long get_broadcast_address(unsigned long ip_address,
                                    unsigned long subnet_mask) {
    return (ip_address & subnet_mask) | (~subnet_mask);
}

unsigned long get_network_id(unsigned long ip_address,
                             unsigned long subnet_mask) {
    return (ip_address & subnet_mask);
}