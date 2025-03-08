#include "utils/net_utils.h"
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <winhttp.h>

void print_routing_table() { system("route print"); }

void print_network_statistics() { system("netstat -e"); }


int get_global_ip(char *ip_buffer, size_t buffer_size) {
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    LPSTR pszOutBuffer = NULL;
    BOOL bResults = FALSE;
    HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
    int result = -1;

    // Initialize the buffer
    if (ip_buffer && buffer_size > 0) {
        ip_buffer[0] = '\0';
    } else {
        return -1;
    }

    // Use WinHTTP to connect to ipify.org API
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

            // We got the IP, copy it to the buffer
            if (dwDownloaded <= buffer_size - 1) {
                strncpy(ip_buffer, pszOutBuffer, dwDownloaded);
                ip_buffer[dwDownloaded] = '\0';
                result = 0;
            }

            free(pszOutBuffer);
        } while (dwSize > 0);
    }

    // Close open handles
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    return result;
}