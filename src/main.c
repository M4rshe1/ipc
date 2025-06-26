#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h> // Required for inet_pton

#include "core/adapter_info.h"
#include "display/display.h"
#include "utils/net_utils.h"
#include "utils/commands.h"
#include "utils/ip_calculator.h"

#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "ws2_32.lib")

void print_usage(char *program_name);

int main(int argc, char *argv[]) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        return 1;
    }

    int show_all = 0;
    int show_ipv4 = 1;
    int show_ipv6 = 1;
    int brief_output = 0;
    int show_details = 0;
    int show_dns = 1;
    int only_physical = 0;
    int hide_sensitive = 0;
    int show_subnet = 0;
    char *resolve_mac = NULL;

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
            {"renew",         optional_argument, 0, 1},
            {"only-physical", no_argument,       0, 'p'},
            {"global",        no_argument,       0, 'g'},
            {"release",       optional_argument, 0, 2},
            {"flush-dns",     no_argument,       0, 'f'},
            {"connections",   no_argument,       0, 'c'},
            {"lookup",        required_argument, 0, 'l'},
            {"hide",          no_argument,       0, 'i'},
            {"resolve-mac",   required_argument, 0, 'm'},
            {"wake",          required_argument, 0, 'w'},
            {"calc",          required_argument, 0, 3},
            {0,               0,                 0, 0},
    };

    while ((opt = getopt_long(argc, argv, "a46bcdhtnrspfcgl:him:w:", long_options,
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
            case 't':
                show_subnet = 1;
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
                print_routing_table();
                WSACleanup();
                return 0;
            case 's':
                print_network_statistics();
                WSACleanup();
                return 0;
            case 'p':
                only_physical = 1;
                break;
            case 'f':
                system("ipconfig /flushdns");
                WSACleanup();
                return 0;
            case 'c':
                system("netstat -an");
                WSACleanup();
                return 0;
            case 'g':
                print_global_ip(hide_sensitive);
                WSACleanup();
                return 0;
            case 'l':
                nslookup(optarg);
                WSACleanup();
                return 0;
            case 'i':
                hide_sensitive = 1;
                break;
            case 'm':
                resolve_mac = optarg;
                break;
            case 'w':
                wake_on_lan(optarg);
                WSACleanup();
                return 0;
            case 1:
                renew_dhcp_lease(optarg);
                WSACleanup();
                return 0;
            case 2:
                release_dhcp_lease(optarg);
                WSACleanup();
                return 0;
            case 3: {
                char *ip_str = strtok(optarg, "/");
                char *mask_str = strtok(NULL, "/");
                if (ip_str && mask_str) {
                    calculate_subnet(ip_str, atoi(mask_str));
                } else {
                    fprintf(stderr, "Invalid format for --calc. Use ip/mask.\n");
                }
                WSACleanup();
                return 0;
            }
            default:
                fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
                WSACleanup();
                return 1;
        }
    }

    PIP_ADAPTER_ADDRESSES pAdapterAddresses = get_adapter_addresses();
    if (pAdapterAddresses == NULL) {
        WSACleanup();
        return 1;
    }
    if (resolve_mac != NULL) {
        char resolved_address[64];

        struct in_addr ip_addr;
        if (inet_pton(AF_INET, resolve_mac, &ip_addr) == 1) {
            if (resolve_mac_address(resolve_mac, resolved_address,
                                    sizeof(resolved_address)) == 0) {
                printf("%s -> %s\n", resolve_mac, resolved_address);
            } else {
                fprintf(stderr, "Failed to resolve MAC address for %s\n",
                        resolve_mac);
            }
        } else {
            if (resolve_ip_address(resolve_mac, resolved_address,
                                   sizeof(resolved_address)) == 0) {
                printf("%s -> %s\n", resolve_mac, resolved_address);
            } else {
                fprintf(stderr, "Failed to resolve IP address for %s\n",
                        resolve_mac);
            }
        }
        WSACleanup();
        return 0;
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
                             brief_output, show_details, show_dns, show_subnet);


    free(pAdapterAddresses);
    WSACleanup();

    return 0;
}

void print_usage(char *program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("Windows IP configuration tool\n\n");
    printf("Options:\n");
    printf("  -a, --all                     Show all adapters (including disconnected ones)\n");
    printf("  -4, --ipv4                    Show only IPv4 addresses\n");
    printf("  -6, --ipv6                    Show only IPv6 addresses\n");
    printf("  -b, --brief                   Brief output format\n");
    printf("  -t                            calculate subnet details for an IP address\n");
    printf("  -c, --connections             Show active network connections\n");
    printf("  -d, --details                 Show detailed information\n");
    printf("  -f, --flush-dns               Flush DNS resolver cache\n");
    printf("  -h, --help                    Display this help message\n");
    printf("  -i, --hide                    Hide sensitive information (e.g., Public IP)\n");
    printf("  -l, --lookup <host>           Lookup DNS information for a host\n");
    printf("  -m, --resolve-mac <mac|ip>    Resolve MAC address for each IP address\n");
    printf("  -n, --no-dns                  Don't show DNS information\n");
    printf("  -p, --only-physical           Show only physical adapters\n");
    printf("  -r, --route                   Show routing table\n");
    printf("  -s, --stats                   Show network statistics\n");
    printf("  -g, --global                  Show global (public) IP address\n");
    printf("  -w, --wake <target>           Send Wake-on-LAN packet to target (MAC or IP)\n");
    printf("      --renew <?adapter>        Renew DHCP lease for adapter\n");
    printf("      --release <?adapter>      Release DHCP lease for adapter");
    printf("      --calc <ip/mask>          Calculate subnet details for an IP address");
    printf("\n");
}




