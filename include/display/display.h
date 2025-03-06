#ifndef DISPLAY_H
#define DISPLAY_H

#include <windows.h>
#include <iphlpapi.h>
#include <iptypes.h>


void display_ip_configuration(PIP_ADAPTER_ADDRESSES pAdapterAddresses,
                              int show_all, int show_ipv4, int show_ipv6,
                              int brief_output, int show_details, int show_dns);

#endif
