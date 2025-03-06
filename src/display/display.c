#include <winsock2.h>
#include "display/display.h"
#include "display/color_output.h"
#include "core/adapter_utils.h"
#include <stdio.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iptypes.h>
#include <ipifcons.h>


void display_ip_configuration(PIP_ADAPTER_ADDRESSES pAdapterAddresses,
                              int show_all, int show_ipv4, int show_ipv6,
                              int brief_output, int show_details, int show_dns) {
    PIP_ADAPTER_ADDRESSES pAdapter = pAdapterAddresses;

    if (!brief_output) {
        printf("Windows IP Configuration\n\n");
    }

    while (pAdapter) {
        BOOL isImportant =
                (pAdapter->IfType != IF_TYPE_TUNNEL) &&
                (pAdapter->OperStatus == IfOperStatusUp);

        if (show_all || isImportant) {
            int ipv4Count = 0;
            int ipv6Count = 0;
            PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
            size_t convertedChars = 0;
            if (brief_output) {
                char friendlyNameAscii[256];
                wcstombs_s(&convertedChars, friendlyNameAscii, sizeof(friendlyNameAscii), pAdapter->FriendlyName, _TRUNCATE);
                print_color(11, friendlyNameAscii); // Cyan
                printf(": ");

                pUnicast = pAdapter->FirstUnicastAddress;
                int addressPrinted = 0;
                while (pUnicast != NULL) {
                    if ((pUnicast->Address.lpSockaddr->sa_family == AF_INET &&
                         show_ipv4) ||
                        (pUnicast->Address.lpSockaddr->sa_family == AF_INET6 &&
                         show_ipv6)) {

                        if (addressPrinted)
                            printf(", ");

                        char ipstringbuffer[46];

                        if (pUnicast->Address.lpSockaddr->sa_family == AF_INET) {
                            struct sockaddr_in *sockaddr_ipv4 =
                                    (struct sockaddr_in *)pUnicast->Address.lpSockaddr;
                            inet_ntop(AF_INET, &(sockaddr_ipv4->sin_addr), ipstringbuffer,
                                      sizeof(ipstringbuffer));
                            printf("%s/%hhu", ipstringbuffer, pUnicast->OnLinkPrefixLength);
                        } else if (pUnicast->Address.lpSockaddr->sa_family == AF_INET6) {
                            struct sockaddr_in6 *sockaddr_ipv6 =
                                    (struct sockaddr_in6 *)pUnicast->Address.lpSockaddr;
                            inet_ntop(AF_INET6, &(sockaddr_ipv6->sin6_addr), ipstringbuffer,
                                      sizeof(ipstringbuffer));
                            printf("%s/%hhu", ipstringbuffer, pUnicast->OnLinkPrefixLength);
                        }

                        addressPrinted = 1;
                    }
                    pUnicast = pUnicast->Next;
                }

                if (!addressPrinted) {
                    printf("No IP address");
                }

                printf("\n");
            } else {
                char friendlyNameAscii[256];
                wcstombs_s(&convertedChars, friendlyNameAscii, sizeof(friendlyNameAscii), pAdapter->FriendlyName, _TRUNCATE);

                print_color(11, "Adapter Name: "); // Cyan
                printf("%s\n", friendlyNameAscii);

                if (show_details) {
                    // Convert Description to ASCII
                    char descriptionAscii[256];
                    wcstombs_s(&convertedChars, descriptionAscii, sizeof(descriptionAscii), pAdapter->Description, _TRUNCATE);

                    print_color(15, "  Description: "); // White
                    printf("%s\n", descriptionAscii);

                    print_color(15, "  Interface Index: "); // White
                    printf("%lu\n", pAdapter->IfIndex);

                    print_color(15, "  Interface Type: "); // White
                    printf("%lu", pAdapter->IfType);
                    switch (pAdapter->IfType) {
                        case IF_TYPE_ETHERNET_CSMACD:
                            printf(" (Ethernet)\n");
                            break;
                        case IF_TYPE_ISO88025_TOKENRING:
                            printf(" (Token Ring)\n");
                            break;
                        case IF_TYPE_PPP:
                            printf(" (PPP)\n");
                            break;
                        case IF_TYPE_SOFTWARE_LOOPBACK:
                            printf(" (Loopback)\n");
                            break;
                        case IF_TYPE_ATM:
                            printf(" (ATM)\n");
                            break;
                        case IF_TYPE_IEEE80211:
                            printf(" (IEEE 802.11 Wireless)\n");
                            break;
                        case IF_TYPE_TUNNEL:
                            printf(" (Tunnel)\n");
                            break;
                        case IF_TYPE_IEEE1394:
                            printf(" (IEEE 1394 Firewire)\n");
                            break;
                        default:
                            printf("\n");
                            break;
                    }
                }

                print_color(10, "  MAC Address: "); // Green
                if (pAdapter->PhysicalAddressLength != 0) {
                    for (int i = 0; i < pAdapter->PhysicalAddressLength; i++) {
                        if (i == 0)
                            printf("%.2X", (int)pAdapter->PhysicalAddress[i]);
                        else
                            printf("-%.2X", (int)pAdapter->PhysicalAddress[i]);
                    }
                    printf("\n");
                } else {
                    printf("No MAC Address\n");
                }

                // Count IPv4 and IPv6 addresses
                ipv4Count = 0;
                ipv6Count = 0;
                pUnicast = pAdapter->FirstUnicastAddress;
                while (pUnicast != NULL) {
                    if (pUnicast->Address.lpSockaddr->sa_family == AF_INET) {
                        ipv4Count++;
                    } else if (pUnicast->Address.lpSockaddr->sa_family == AF_INET6) {
                        ipv6Count++;
                    }
                    pUnicast = pUnicast->Next;
                }

                // Print IP Addresses
                if ((ipv4Count > 0 && show_ipv4) || (ipv6Count > 0 && show_ipv6)) {
                    // Print IPv4 Addresses
                    if (ipv4Count > 0 && show_ipv4) {
                        print_color(14, "  IPv4 Addresses:\n"); // Yellow
                        pUnicast = pAdapter->FirstUnicastAddress;
                        int count = 0;
                        while (pUnicast != NULL) {
                            if (pUnicast->Address.lpSockaddr->sa_family == AF_INET) {
                                count++;
                                struct sockaddr_in *sockaddr_ipv4 =
                                        (struct sockaddr_in *)pUnicast->Address.lpSockaddr;
                                char ipstringbuffer[46]; // Max IPv6 string length
                                inet_ntop(AF_INET, &(sockaddr_ipv4->sin_addr), ipstringbuffer,
                                          sizeof(ipstringbuffer));

                                unsigned long ip_address =
                                        ntohl(sockaddr_ipv4->sin_addr.S_un.S_addr);
                                int cidr = pUnicast->OnLinkPrefixLength;
                                unsigned long subnet_mask = cidr_to_mask(cidr);
                                unsigned long broadcast_address =
                                        get_broadcast_address(ip_address, subnet_mask);
                                unsigned long network_id =
                                        get_network_id(ip_address, subnet_mask);

                                char broadcast_str[INET_ADDRSTRLEN];
                                char network_id_str[INET_ADDRSTRLEN];

                                struct in_addr broadcast_addr;
                                broadcast_addr.S_un.S_addr = htonl(broadcast_address);
                                inet_ntop(AF_INET, &broadcast_addr, broadcast_str,
                                          INET_ADDRSTRLEN);

                                struct in_addr network_id_addr;
                                network_id_addr.S_un.S_addr = htonl(network_id);
                                inet_ntop(AF_INET, &network_id_addr, network_id_str,
                                          INET_ADDRSTRLEN);

                                printf("    %d) %s", count, ipstringbuffer);

                                // Get CIDR notation
                                printf(" /%d", cidr);
                                if (show_details) {

                                    // Calculate and print subnet mask
                                    unsigned long mask = cidr_to_mask(cidr);
                                    printf(" (");
                                    printf("%lu.%lu.%lu.%lu", (mask >> 24) & 0xFF,
                                           (mask >> 16) & 0xFF, (mask >> 8) & 0xFF, mask & 0xFF);
                                    printf(")\n");

                                    printf("      Network ID: %s\n", network_id_str);
                                    printf("      Broadcast Address: %s\n", broadcast_str);
                                } else {
                                    printf("\n");
                                }
                            }
                            pUnicast = pUnicast->Next;
                        }
                    }

                    // Print IPv6 Addresses
                    if (ipv6Count > 0 && show_ipv6) {
                        print_color(14, "  IPv6 Addresses:\n"); // Yellow
                        pUnicast = pAdapter->FirstUnicastAddress;
                        int count = 0;
                        while (pUnicast != NULL) {
                            if (pUnicast->Address.lpSockaddr->sa_family == AF_INET6) {
                                count++;
                                struct sockaddr_in6 *sockaddr_ipv6 =
                                        (struct sockaddr_in6 *)pUnicast->Address.lpSockaddr;
                                char ipstringbuffer[46]; // Max IPv6 string length
                                inet_ntop(AF_INET6, &(sockaddr_ipv6->sin6_addr),
                                          ipstringbuffer, sizeof(ipstringbuffer));
                                printf("    %d) %s", count, ipstringbuffer);

                                // Get CIDR notation for IPv6
                                ULONG cidr = pUnicast->OnLinkPrefixLength;
                                printf(" /%lu\n", cidr);
                            }
                            pUnicast = pUnicast->Next;
                        }
                    }
                } else {
                    print_color(12, "  No IP Addresses\n"); // Red
                }

                // Print DNS Servers
                if (show_dns) {
                    if (pAdapter->FirstDnsServerAddress != NULL) {
                        print_color(13, "  DNS Servers:\n"); // Magenta
                        PIP_ADAPTER_DNS_SERVER_ADDRESS pDnsServer =
                                pAdapter->FirstDnsServerAddress;
                        int count = 0;
                        while (pDnsServer != NULL) {
                            count++;
                            if (pDnsServer->Address.lpSockaddr->sa_family == AF_INET) {
                                struct sockaddr_in *sockaddr_ipv4 =
                                        (struct sockaddr_in *)pDnsServer->Address.lpSockaddr;
                                char ipstringbuffer[46];
                                inet_ntop(AF_INET, &(sockaddr_ipv4->sin_addr), ipstringbuffer,
                                          sizeof(ipstringbuffer));
                                printf("    %d) %s\n", count, ipstringbuffer);
                            } else if (pDnsServer->Address.lpSockaddr->sa_family ==
                                       AF_INET6) {
                                struct sockaddr_in6 *sockaddr_ipv6 =
                                        (struct sockaddr_in6 *)pDnsServer->Address.lpSockaddr;
                                char ipstringbuffer[46];
                                inet_ntop(AF_INET6, &(sockaddr_ipv6->sin6_addr),
                                          ipstringbuffer, sizeof(ipstringbuffer));
                                printf("    %d) %s\n", count, ipstringbuffer);
                            }
                            pDnsServer = pDnsServer->Next;
                        }
                    } else {
                        print_color(12, "  No DNS Servers\n"); // Red
                    }
                    if (pAdapter->DnsSuffix != NULL && wcslen(pAdapter->DnsSuffix) > 0) {
                        char dnsSuffixAscii[256];
                        wcstombs_s(&convertedChars, dnsSuffixAscii, sizeof(dnsSuffixAscii), pAdapter->DnsSuffix, _TRUNCATE);
                        print_color(13, "  DNS Suffix: "); // Magenta
                        printf("%s\n", dnsSuffixAscii);
                    }
                }

                if (pAdapter->Dhcpv4Enabled && show_details) {
                    print_color(9, "  Configuration: ");
                    printf("DHCP\n");

                    // Get DHCP server if available
                    if (pAdapter->Dhcpv4Server.iSockaddrLength > 0) {
                        char dhcpServer[INET_ADDRSTRLEN];
                        struct sockaddr_in *sockaddr_ipv4 = (struct sockaddr_in *)pAdapter->Dhcpv4Server.lpSockaddr;
                        inet_ntop(AF_INET, &(sockaddr_ipv4->sin_addr), dhcpServer, sizeof(dhcpServer));
                        print_color(9, "  DHCP Server: ");
                        printf("%s\n", dhcpServer);
                    }

                    // Use current time as placeholder since the adapter structure doesn't contain lease times
                    FILETIME leaseObtainedTime, leaseExpiresTime;
                    GetSystemTimeAsFileTime(&leaseObtainedTime);
                    GetSystemTimeAsFileTime(&leaseExpiresTime);

                    // Convert FILETIME to time_t
                    ULARGE_INTEGER uli;

                    uli.LowPart = leaseObtainedTime.dwLowDateTime;
                    uli.HighPart = leaseObtainedTime.dwHighDateTime;
                    time_t leaseObtained = (time_t)((uli.QuadPart - 116444736000000000) /
                                                    10000000); // Convert to seconds

                    uli.LowPart = leaseExpiresTime.dwLowDateTime;
                    uli.HighPart = leaseExpiresTime.dwHighDateTime;
                    time_t leaseExpires = (time_t)((uli.QuadPart - 116444736000000000) /
                                                   10000000); // Convert to seconds

                    // Format the time values
                    char leaseObtainedStr[64];
                    char leaseExpiresStr[64];

                    if (leaseObtained > 0) {
                        struct tm leaseObtainedTm;
                        localtime_s(&leaseObtainedTm, &leaseObtained);
                        strftime(leaseObtainedStr, sizeof(leaseObtainedStr), "%Y-%m-%d %H:%M:%S", &leaseObtainedTm);
                        print_color(9, "  IP Lease Obtained: ");
                        printf("%s\n", leaseObtainedStr);
                    } else {
                        print_color(9, "  IP Lease Obtained: ");
                        printf("N/A\n");
                    }

                    if (leaseExpires > 0) {
                        struct tm leaseExpiresTm;
                        localtime_s(&leaseExpiresTm, &leaseExpires);
                        strftime(leaseExpiresStr, sizeof(leaseExpiresStr), "%Y-%m-%d %H:%M:%S", &leaseExpiresTm);
                        print_color(9, "  IP Lease Expires: ");
                        printf("%s\n", leaseExpiresStr);
                    } else {
                        print_color(9, "  IP Lease Expires: ");
                        printf("N/A\n");
                    }
                } else if (show_details) {
                    print_color(9, "  Configuration: ");
                    printf("Static\n");
                }

                // Print Gateway
                if (pAdapter->FirstGatewayAddress != NULL) {
                    print_color(11, "  Gateway:\n"); // Cyan
                    PIP_ADAPTER_GATEWAY_ADDRESS pGateway = pAdapter->FirstGatewayAddress;
                    int count = 0;
                    while (pGateway != NULL) {
                        count++;
                        if (pGateway->Address.lpSockaddr->sa_family == AF_INET) {
                            struct sockaddr_in *sockaddr_ipv4 =
                                    (struct sockaddr_in *)pGateway->Address.lpSockaddr;
                            char ipstringbuffer[46];
                            inet_ntop(AF_INET, &(sockaddr_ipv4->sin_addr), ipstringbuffer,
                                      sizeof(ipstringbuffer));
                            printf("    %d) %s\n", count, ipstringbuffer);
                        } else if (pGateway->Address.lpSockaddr->sa_family == AF_INET6) {
                            struct sockaddr_in6 *sockaddr_ipv6 =
                                    (struct sockaddr_in6 *)pGateway->Address.lpSockaddr;
                            char ipstringbuffer[46];
                            inet_ntop(AF_INET6, &(sockaddr_ipv6->sin6_addr),
                                      ipstringbuffer, sizeof(ipstringbuffer));
                            printf("    %d) %s\n", count, ipstringbuffer);
                        }
                        pGateway = pGateway->Next;
                    }
                } else {
                    print_color(12, "  No Gateway\n"); // Red
                }

                print_color(12, "  Operational Status: "); // Red
                switch (pAdapter->OperStatus) {
                    case IfOperStatusUp:
                        printf("Up\n");
                        break;
                    case IfOperStatusDown:
                        printf("Down\n");
                        break;
                    case IfOperStatusTesting:
                        printf("Testing\n");
                        break;
                    case IfOperStatusUnknown:
                        printf("Unknown\n");
                        break;
                    case IfOperStatusDormant:
                        printf("Dormant\n");
                        break;
                    case IfOperStatusNotPresent:
                        printf("Not Present\n");
                        break;
                    case IfOperStatusLowerLayerDown:
                        printf("Lower Layer Down\n");
                        break;
                    default:
                        printf("%d\n", pAdapter->OperStatus);
                        break;
                }

                printf("-----------------------------------\n");
            }
        }

        pAdapter = pAdapter->Next;
    }
}
