#include <winsock2.h>
#include "display/display.h"
#include "display/color_output.h"
#include <stdio.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iptypes.h>
#include <ipifcons.h>

void display_ip_configuration(PIP_ADAPTER_ADDRESSES pAdapterAddresses,
                              int show_all, int show_ipv4, int show_ipv6,
                              int brief_output, int show_details, int show_dns) {
    if (!brief_output) {
        printf("Windows IP Configuration\n\n");
    }

    for (PIP_ADAPTER_ADDRESSES pAdapter = pAdapterAddresses; pAdapter; pAdapter = pAdapter->Next) {
        if (show_all || (pAdapter->IfType != IF_TYPE_TUNNEL && pAdapter->OperStatus == IfOperStatusUp)) {
            size_t convertedChars = 0;
            char friendlyNameAscii[256];
            wcstombs_s(&convertedChars, friendlyNameAscii, sizeof(friendlyNameAscii), pAdapter->FriendlyName,
                       _TRUNCATE);

            if (brief_output) {
                print_color(11, friendlyNameAscii); // Cyan
                printf(": ");

                int addressPrinted = 0;
                for (PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pAdapter->FirstUnicastAddress; pUnicast; pUnicast = pUnicast->Next) {
                    if ((pUnicast->Address.lpSockaddr->sa_family == AF_INET && show_ipv4) ||
                        (pUnicast->Address.lpSockaddr->sa_family == AF_INET6 && show_ipv6)) {

                        if (addressPrinted) printf(", ");

                        char ipstringbuffer[46];
                        if (pUnicast->Address.lpSockaddr->sa_family == AF_INET) {
                            struct sockaddr_in *sockaddr_ipv4 = (struct sockaddr_in *) pUnicast->Address.lpSockaddr;
                            inet_ntop(AF_INET, &(sockaddr_ipv4->sin_addr), ipstringbuffer, sizeof(ipstringbuffer));
                        } else {
                            struct sockaddr_in6 *sockaddr_ipv6 = (struct sockaddr_in6 *) pUnicast->Address.lpSockaddr;
                            inet_ntop(AF_INET6, &(sockaddr_ipv6->sin6_addr), ipstringbuffer, sizeof(ipstringbuffer));
                        }
                        printf("%s/%hhu", ipstringbuffer, pUnicast->OnLinkPrefixLength);
                        addressPrinted = 1;
                    }
                }
                if (!addressPrinted) printf("No IP address");
                printf("\n");
            } else {
                print_color(11, "Adapter Name: "); // Cyan
                printf("%s\n", friendlyNameAscii);

                if (show_details) {
                    char descriptionAscii[256];
                    wcstombs_s(&convertedChars, descriptionAscii, sizeof(descriptionAscii), pAdapter->Description,
                               _TRUNCATE);
                    print_color(15, "  Description: "); // White
                    printf("%s\n", descriptionAscii);
                    print_color(15, "  Interface Index: "); // White
                    printf("%lu\n", pAdapter->IfIndex);
                    print_color(15, "  Interface Type: "); // White
                    printf("%lu\n", pAdapter->IfType);
                }

                print_color(10, "  MAC Address: "); // Green
                if (pAdapter->PhysicalAddressLength != 0) {
                    for (unsigned int i = 0; i < pAdapter->PhysicalAddressLength; i++) {
                        if (i > 0) printf("-");
                        printf("%.2X", (int) pAdapter->PhysicalAddress[i]);
                    }
                    printf("\n");
                } else {
                    printf("No MAC Address\n");
                }

                int ipv4Count = 0, ipv6Count = 0;
                for (PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pAdapter->FirstUnicastAddress; pUnicast; pUnicast = pUnicast->Next) {
                    if (pUnicast->Address.lpSockaddr->sa_family == AF_INET) ipv4Count++;
                    else if (pUnicast->Address.lpSockaddr->sa_family == AF_INET6) ipv6Count++;
                }

                if ((ipv4Count > 0 && show_ipv4) || (ipv6Count > 0 && show_ipv6)) {
                    if (ipv4Count > 0 && show_ipv4) {
                        print_color(14, "  IPv4 Addresses:\n"); // Yellow
                        int count = 0;
                        for (PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pAdapter->FirstUnicastAddress; pUnicast; pUnicast = pUnicast->Next) {
                            if (pUnicast->Address.lpSockaddr->sa_family == AF_INET) {
                                count++;
                                struct sockaddr_in *sockaddr_ipv4 = (struct sockaddr_in *) pUnicast->Address.lpSockaddr;
                                char ipstringbuffer[46];
                                inet_ntop(AF_INET, &(sockaddr_ipv4->sin_addr), ipstringbuffer, sizeof(ipstringbuffer));
                                printf("    %d) %s /%d\n", count, ipstringbuffer, pUnicast->OnLinkPrefixLength);
                            }
                        }
                    }
                    if (ipv6Count > 0 && show_ipv6) {
                        print_color(14, "  IPv6 Addresses:\n"); // Yellow
                        int count = 0;
                        for (PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pAdapter->FirstUnicastAddress; pUnicast; pUnicast = pUnicast->Next) {
                            if (pUnicast->Address.lpSockaddr->sa_family == AF_INET6) {
                                count++;
                                struct sockaddr_in6 *sockaddr_ipv6 = (struct sockaddr_in6 *) pUnicast->Address.lpSockaddr;
                                char ipstringbuffer[46];
                                inet_ntop(AF_INET6, &(sockaddr_ipv6->sin6_addr), ipstringbuffer,
                                          sizeof(ipstringbuffer));
                                printf("    %d) %s /%lu\n", count, ipstringbuffer, pUnicast->OnLinkPrefixLength);
                            }
                        }
                    }
                } else {
                    print_color(12, "  No IP Addresses\n"); // Red
                }

                if (show_dns) {
                    if (pAdapter->FirstDnsServerAddress) {
                        print_color(13, "  DNS Servers:\n"); // Magenta
                        int count = 0;
                        for (PIP_ADAPTER_DNS_SERVER_ADDRESS pDnsServer = pAdapter->FirstDnsServerAddress; pDnsServer; pDnsServer = pDnsServer->Next) {
                            count++;
                            char ipstringbuffer[46];
                            if (pDnsServer->Address.lpSockaddr->sa_family == AF_INET) {
                                struct sockaddr_in *sockaddr_ipv4 = (struct sockaddr_in *) pDnsServer->Address.lpSockaddr;
                                inet_ntop(AF_INET, &(sockaddr_ipv4->sin_addr), ipstringbuffer, sizeof(ipstringbuffer));
                            } else {
                                struct sockaddr_in6 *sockaddr_ipv6 = (struct sockaddr_in6 *) pDnsServer->Address.lpSockaddr;
                                inet_ntop(AF_INET6, &(sockaddr_ipv6->sin6_addr), ipstringbuffer,
                                          sizeof(ipstringbuffer));
                            }
                            printf("    %d) %s\n", count, ipstringbuffer);
                        }
                    } else {
                        print_color(12, "  No DNS Servers\n"); // Red
                    }
                    if (pAdapter->DnsSuffix && wcslen(pAdapter->DnsSuffix) > 0) {
                        char dnsSuffixAscii[256];
                        wcstombs_s(&convertedChars, dnsSuffixAscii, sizeof(dnsSuffixAscii), pAdapter->DnsSuffix,
                                   _TRUNCATE);
                        print_color(13, "  DNS Suffix: "); // Magenta
                        printf("%s\n", dnsSuffixAscii);
                    }
                }

                if (pAdapter->Dhcpv4Enabled && show_details) {
                    print_color(9, "  Configuration: ");
                    printf("DHCP\n");
                    if (pAdapter->Dhcpv4Server.iSockaddrLength > 0) {
                        char dhcpServer[INET_ADDRSTRLEN];
                        struct sockaddr_in *sockaddr_ipv4 = (struct sockaddr_in *) pAdapter->Dhcpv4Server.lpSockaddr;
                        inet_ntop(AF_INET, &(sockaddr_ipv4->sin_addr), dhcpServer, sizeof(dhcpServer));
                        print_color(9, "  DHCP Server: ");
                        printf("%s\n", dhcpServer);
                    }

                    PIP_ADAPTER_INFO pAdapterInfo = NULL;
                    ULONG ulOutBufLen = 0;
                    if (GetAdaptersInfo(NULL, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
                        pAdapterInfo = (IP_ADAPTER_INFO *) malloc(ulOutBufLen);
                        if (pAdapterInfo && GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == NO_ERROR) {
                            for (PIP_ADAPTER_INFO pAdapterInfoCurrent = pAdapterInfo; pAdapterInfoCurrent; pAdapterInfoCurrent = pAdapterInfoCurrent->Next) {
                                if (pAdapterInfoCurrent->Index == pAdapter->IfIndex) {
                                    char leaseObtainedStr[64] = "N/A", leaseExpiresStr[64] = "N/A";
                                    if (pAdapterInfoCurrent->LeaseObtained > 0) {
                                        struct tm leaseObtainedTm;
                                        localtime_s(&leaseObtainedTm, &pAdapterInfoCurrent->LeaseObtained);
                                        strftime(leaseObtainedStr, sizeof(leaseObtainedStr), "%Y-%m-%d %H:%M:%S",
                                                 &leaseObtainedTm);
                                    }
                                    if (pAdapterInfoCurrent->LeaseExpires > 0) {
                                        struct tm leaseExpiresTm;
                                        localtime_s(&leaseExpiresTm, &pAdapterInfoCurrent->LeaseExpires);
                                        strftime(leaseExpiresStr, sizeof(leaseExpiresStr), "%Y-%m-%d %H:%M:%S",
                                                 &leaseExpiresTm);
                                    }
                                    print_color(9, "  IP Lease Obtained: ");
                                    printf("%s\n", leaseObtainedStr);
                                    print_color(9, "  IP Lease Expires: ");
                                    printf("%s\n", leaseExpiresStr);
                                    break;
                                }
                            }
                            free(pAdapterInfo);
                        }
                    }
                } else if (show_details) {
                    print_color(9, "  Configuration: ");
                    printf("Static\n");
                }

                if (pAdapter->FirstGatewayAddress) {
                    print_color(11, "  Gateway:\n"); // Cyan
                    int count = 0;
                    for (PIP_ADAPTER_GATEWAY_ADDRESS pGateway = pAdapter->FirstGatewayAddress; pGateway; pGateway = pGateway->Next) {
                        count++;
                        char ipstringbuffer[46];
                        if (pGateway->Address.lpSockaddr->sa_family == AF_INET) {
                            struct sockaddr_in *sockaddr_ipv4 = (struct sockaddr_in *) pGateway->Address.lpSockaddr;
                            inet_ntop(AF_INET, &(sockaddr_ipv4->sin_addr), ipstringbuffer, sizeof(ipstringbuffer));
                        } else {
                            struct sockaddr_in6 *sockaddr_ipv6 = (struct sockaddr_in6 *) pGateway->Address.lpSockaddr;
                            inet_ntop(AF_INET6, &(sockaddr_ipv6->sin6_addr), ipstringbuffer, sizeof(ipstringbuffer));
                        }
                        printf("    %d) %s\n", count, ipstringbuffer);
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
    }
}