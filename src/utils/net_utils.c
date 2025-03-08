#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <winhttp.h>
#include <stdio.h>
#include <iphlpapi.h>

#include "utils/net_utils.h"


int get_global_ip(char *ip_buffer, size_t buffer_size) {
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    LPSTR pszOutBuffer = NULL;
    BOOL bResults = FALSE;
    HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
    int result = -1;

    if (ip_buffer && buffer_size > 0) {
        ip_buffer[0] = '\0';
    } else {
        return -1;
    }

    hSession = WinHttpOpen(L"IP Tool/1.0",
                           WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                           WINHTTP_NO_PROXY_NAME,
                           WINHTTP_NO_PROXY_BYPASS, 0);

    if (hSession) {
        hConnect = WinHttpConnect(hSession, L"api.ipify.org",
                                  INTERNET_DEFAULT_HTTPS_PORT, 0);
    }

    if (hConnect) {
        hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/?format=text",
                                      NULL, WINHTTP_NO_REFERER,
                                      WINHTTP_DEFAULT_ACCEPT_TYPES,
                                      WINHTTP_FLAG_SECURE);
    }

    if (hRequest) {
        bResults = WinHttpSendRequest(hRequest,
                                      WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                      WINHTTP_NO_REQUEST_DATA, 0,
                                      0, 0);
    }

    if (bResults) {
        bResults = WinHttpReceiveResponse(hRequest, NULL);
    }

    if (bResults) {
        do {
            dwSize = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
                break;
            }

            if (dwSize == 0) {
                break;
            }

            pszOutBuffer = malloc(dwSize + 1);
            if (!pszOutBuffer) {
                break;
            }

            ZeroMemory(pszOutBuffer, dwSize + 1);

            if (!WinHttpReadData(hRequest, pszOutBuffer,
                                 dwSize, &dwDownloaded)) {
                free(pszOutBuffer);
                break;
            }

            if (dwDownloaded <= buffer_size - 1) {
                strncpy(ip_buffer, pszOutBuffer, dwDownloaded);
                ip_buffer[dwDownloaded] = '\0';
                result = 0;
            }

            free(pszOutBuffer);
        } while (dwSize > 0);
    }

    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    return result;
}

void print_global_ip(int hide_sensitive) {
    char global_ip[100];
    if (get_global_ip(global_ip, sizeof(global_ip)) == 0) {
        if (!hide_sensitive) {
            printf("Global IP Address: %s\n", global_ip);
        } else {
            printf("Global IP Address: [hidden]\n");
        }
        if (OpenClipboard(NULL)) {
            EmptyClipboard();
            HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, strlen(global_ip) + 1);
            memcpy(GlobalLock(hGlob), global_ip, strlen(global_ip) + 1);
            GlobalUnlock(hGlob);
            SetClipboardData(CF_TEXT, hGlob);
            CloseClipboard();
        }
    } else {
        printf("Failed to retrieve global IP address\n");
    }
}

int wake_on_lan(const char *target) {
    SOCKET sock;
    SOCKADDR_IN addr;
    unsigned char packet[102];
    unsigned char mac[6];
    int i;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed in wake_on_lan.\n");
        return -1;
    }

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed: %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    // Determine if target is an IP address or MAC address
    struct in_addr ip_addr;
    char target_ip[16] = {0};
    char mac_address_buffer[18] = {0};

    if (inet_pton(AF_INET, target, &ip_addr) == 1) {
        // Target is an IP address, resolve to MAC
        strcpy(target_ip, target);
        if (resolve_mac_address(target, mac_address_buffer, sizeof(mac_address_buffer)) != 0) {
            fprintf(stderr, "Failed to resolve MAC address for IP: %s\n", target);
            closesocket(sock);
            WSACleanup();
            return -1;
        }

        if (sscanf(mac_address_buffer, "%02hhX-%02hhX-%02hhX-%02hhX-%02hhX-%02hhX",
                   &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) != 6) {
            fprintf(stderr, "Invalid MAC address format: %s\n", mac_address_buffer);
            closesocket(sock);
            WSACleanup();
            return -1;
        }
        printf("Resolved IP %s to MAC %s\n", target, mac_address_buffer);
    } else {
        // Target is a MAC address
        if (sscanf(target, "%02hhX-%02hhX-%02hhX-%02hhX-%02hhX-%02hhX",
                   &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) != 6) {
            fprintf(stderr, "Invalid MAC address format: %s\n", target);
            closesocket(sock);
            WSACleanup();
            return -1;
        }
        strcpy(mac_address_buffer, target);
    }

    // Create the magic packet
    for (i = 0; i < 6; i++) {
        packet[i] = 0xFF;
    }
    for (i = 0; i < 16; i++) {
        memcpy(packet + 6 + i * 6, mac, 6);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9);
    addr.sin_addr.s_addr = inet_addr("255.255.255.255");

    BOOL bOptVal = TRUE;
    int bOptLen = sizeof(BOOL);
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&bOptVal, bOptLen) == SOCKET_ERROR) {
        fprintf(stderr, "setsockopt failed: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    printf("Sending Wake-on-LAN packet to MAC: %s\n", mac_address_buffer);

    if (sendto(sock, (const char *)packet, sizeof(packet), 0,
               (SOCKADDR *)&addr, sizeof(addr)) == SOCKET_ERROR) {
        fprintf(stderr, "sendto failed: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    printf("Wake-on-LAN packet sent successfully\n");
    closesocket(sock);
    WSACleanup();
    return 0;
}


int resolve_mac_address(const char *ip_address, char *mac_address_buffer,
                        int buffer_size) {
    DWORD mac_address[2];
    ULONG mac_address_len = 6;
    IPAddr ip_address_numeric;

    if (inet_pton(AF_INET, ip_address, &ip_address_numeric) != 1) {
        fprintf(stderr, "Invalid IP address format: %s\n", ip_address);
        return -1;
    }

    DWORD result = SendARP(ip_address_numeric, 0, mac_address, &mac_address_len);
    if (result != NO_ERROR) {
        fprintf(stderr, "SendARP failed with error: %lu\n", result);
        return -1;
    }

    snprintf(mac_address_buffer, buffer_size, "%02X-%02X-%02X-%02X-%02X-%02X",
             (BYTE) mac_address[0], (BYTE) (mac_address[0] >> 8),
             (BYTE) (mac_address[0] >> 16), (BYTE) mac_address[1],
             (BYTE) (mac_address[1] >> 8), (BYTE) (mac_address[1] >> 16));

    return 0;
}

int resolve_ip_address(const char *mac_address, char *ip_address_buffer,
                       int buffer_size) {
    unsigned char mac[6];
    char *ptr;
    const char *mac_ptr = mac_address;
    int i;

    for (i = 0; i < 6; i++) {
        mac[i] = (unsigned char) strtoul(mac_ptr, &ptr, 16);
        if (mac[i] > 255) {
            fprintf(stderr, "Invalid MAC address value at byte %d\n", i);
            return -1;
        }
        if (i < 5) {
            if (*ptr != '-') {
                fprintf(stderr, "Invalid MAC address format: missing '-' at byte %d\n",
                        i);
                return -1;
            }
            mac_ptr = ptr + 1;
        } else if (*ptr != '\0') {
            fprintf(stderr, "Invalid MAC address format: extra characters\n");
            return -1;
        }
    }

    MIB_IPNETTABLE *pIpNetTable = NULL;
    DWORD dwSize = 0;
    DWORD dwResult = GetIpNetTable(NULL, &dwSize, FALSE);
    if (dwResult == ERROR_INSUFFICIENT_BUFFER) {
        pIpNetTable = (MIB_IPNETTABLE *) malloc(dwSize);
        if (pIpNetTable == NULL) {
            fprintf(stderr, "Failed to allocate memory for ARP table\n");
            return -1;
        }
        dwResult = GetIpNetTable(pIpNetTable, &dwSize, FALSE);
    }

    if (dwResult != NO_ERROR) {
        fprintf(stderr, "GetIpNetTable failed with error: %lu\n", dwResult);
        if (pIpNetTable) {
            free(pIpNetTable);
        }
        return -1;
    }
    if (pIpNetTable == NULL) {
        fprintf(stderr, "No ARP entries found\n");
        return 1;
    }
    for (i = 0; i < (int) pIpNetTable->dwNumEntries; i++) {
        MIB_IPNETROW row = pIpNetTable->table[i];
        if (row.dwPhysAddrLen == 6 && memcmp(row.bPhysAddr, mac, 6) == 0) {
            struct in_addr ip_addr;
            ip_addr.S_un.S_addr = row.dwAddr;
            inet_ntop(AF_INET, &ip_addr, ip_address_buffer, buffer_size);
            free(pIpNetTable);
            return 0;
        }
    }

    fprintf(stderr, "No IP address found for MAC address: %s\n", mac_address);
    free(pIpNetTable);
    return -1;
}