#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <winsock2.h>
#include <iphlpapi.h>

#include "core/adapter_info.h"
#include "display/display.h"
#include "utils/net_utils.h"
#include "utils/ipconfig_commands.h"

#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "ws2_32.lib")

void print_usage(char *program_name);

int main(int argc, char *argv[]) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        return 1;
    }

    // Option flags
    int show_all = 0;
    int show_ipv4 = 1;
    int show_ipv6 = 1;
    int brief_output = 0;
    int show_details = 0;
    int show_dns = 1;
    int only_physical = 0;
    int show_route = 0;
    int show_stats = 0;
    char *renew_adapter = NULL;
    char *release_adapter = NULL;
    int flush_dns = 0;
    int show_connections = 0;

    int opt;
    int option_index = 0;
    static struct option long_options[] = {
            {"all",           no_argument,       0, 'a'},
            {"ipv4",          no_argument,       0, '4'},
            {"ipv6",          no_argument,       0, '6'},
            {"brief",         no_argument,       0, 'b'},
            {"details",       no_argument,       0, 'd'},
            {"help",          no_argument,       0, 'h'},
            {"no-dns",        no_argument,       0, 'n'},
            {"route",         no_argument,       0, 'r'},
            {"stats",         no_argument,       0, 's'},
            {"renew",         required_argument, 0, 1},
            {"only-physical", no_argument,       0, 'p'},
            {"release",       required_argument, 0, 2},
            {"flush-dns",     no_argument,       0, 'f'},
            {"connections",   no_argument,       0, 'c'},
            {0,               0,                 0, 0},
    };

    while ((opt = getopt_long(argc, argv, "a46bcdhnrspfc", long_options,
                              &option_index)) != -1) {
        switch (opt) {
            case 'a':
                show_all = 1;
                break;
            case '4':
                show_ipv4 = 1;
                show_ipv6 = 0;
                break;
            case '6':
                show_ipv4 = 0;
                show_ipv6 = 1;
                break;
            case 'b':
                brief_output = 1;
                break;
            case 'd':
                show_details = 1;
                break;
            case 'h':
                print_usage(argv[0]);
                WSACleanup();
                return 0;
            case 'n':
                show_dns = 0;
                break;
            case 'r':
                show_route = 1;
                break;
            case 's':
                show_stats = 1;
                break;
            case 'p':
                only_physical = 1;
                break;
            case 'f':
                flush_dns = 1;
                break;
            case 'c':
                show_connections = 1;
                break;
            case 1:
                renew_adapter = optarg;
                break;
            case 2:
                release_adapter = optarg;
                break;
            default:
                fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
                WSACleanup();
                return 1;
        }
    }

    if (renew_adapter != NULL) {
        renew_dhcp_lease(renew_adapter);
        WSACleanup();
        return 0;
    }

    if (release_adapter != NULL) {
        release_dhcp_lease(release_adapter);
        WSACleanup();
        return 0;
    }

    if (flush_dns) {
        system("ipconfig /flushdns");
        printf("DNS cache flushed successfully.\n");
        WSACleanup();
        return 0;
    }

    if (show_route) {
        print_routing_table();
        WSACleanup();
        return 0;
    }

    if (show_stats) {
        print_network_statistics();
        WSACleanup();
        return 0;
    }

    if (show_connections) {
        system("netstat -an");
        WSACleanup();
        return 0;
    }


    PIP_ADAPTER_ADDRESSES pAdapterAddresses = get_adapter_addresses();
    if (pAdapterAddresses == NULL) {
        WSACleanup();
        return 1;
    }

    if (only_physical) {
        PIP_ADAPTER_ADDRESSES pCurrent = pAdapterAddresses;
        PIP_ADAPTER_ADDRESSES pPrev = NULL;

        while (pCurrent) {
            if (pCurrent->IfType != IF_TYPE_ETHERNET_CSMACD &&
                pCurrent->IfType != IF_TYPE_IEEE80211 &&
                pCurrent->IfType != IF_TYPE_IEEE1394 ||
                pCurrent->IfType == IF_TYPE_SOFTWARE_LOOPBACK ||
                (wcsstr(pCurrent->Description, L"VMware") != NULL) ||
                (wcsstr(pCurrent->Description, L"Loopback") != NULL) ||
                (wcsstr(pCurrent->Description, L"Virtual") != NULL) ||
                (wcsstr(pCurrent->Description, L"VirtualBox") != NULL) ||
                (wcsstr(pCurrent->Description, L"Hyper-V") != NULL)) {

                PIP_ADAPTER_ADDRESSES pNext = pCurrent->Next;

                if (pPrev == NULL) {
                    pAdapterAddresses = pNext;
                } else {
                    pPrev->Next = pNext;
                }

                pCurrent = pNext;
            } else {
                pPrev = pCurrent;
                pCurrent = pCurrent->Next;
            }
        }
    }

    display_ip_configuration(pAdapterAddresses, show_all, show_ipv4, show_ipv6,
                             brief_output, show_details, show_dns);

    free(pAdapterAddresses);
    WSACleanup();

    return 0;
}

void print_usage(char *program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("Windows IP configuration tool\n\n");
    printf("Options:\n");
    printf("  -a, --all             Show all adapters (including disconnected ones)\n");
    printf("  -4, --ipv4            Show only IPv4 addresses\n");
    printf("  -6, --ipv6            Show only IPv6 addresses\n");
    printf("  -b, --brief           Brief output format\n");
    printf("  -c, --connections     Show active network connections\n");
    printf("  -d, --details         Show detailed information\n");
    printf("  -f, --flush-dns       Flush DNS resolver cache\n");
    printf("  -h, --help            Display this help message\n");
    printf("  -n, --no-dns          Don't show DNS information\n");
    printf("  -p, --only-physical   Show only physical adapters\n");
    printf("  -r, --route           Show routing table\n");
    printf("  -s, --stats           Show network statistics\n");
    printf("      --renew <adapter> Renew DHCP lease for adapter\n");
    printf("      --release <adapter> Release DHCP lease for adapter\n");
    printf("\n");
}
