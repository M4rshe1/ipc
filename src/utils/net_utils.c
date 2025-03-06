#include "utils/net_utils.h"
#include <stdlib.h>

void print_routing_table() { system("route print"); }

void print_network_statistics() { system("netstat -e"); }
