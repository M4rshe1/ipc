#include <winsock2.h>
#include "core/adapter_info.h"
#include <stdio.h>
#include <stdlib.h>
#include <iphlpapi.h>

PIP_ADAPTER_ADDRESSES get_adapter_addresses() {
    PIP_ADAPTER_ADDRESSES pAdapterAddresses = NULL;
    ULONG family = AF_UNSPEC;
    DWORD flags = GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_INCLUDE_GATEWAYS;
    ULONG outBufLen = 0;
    DWORD dwRetVal = 0;

    outBufLen = 15000;

    pAdapterAddresses = (IP_ADAPTER_ADDRESSES *)malloc(outBufLen);
    if (pAdapterAddresses == NULL) {
        printf("Memory allocation failed for IP_ADAPTER_ADDRESSES\n");
        return NULL;
    }

    dwRetVal = GetAdaptersAddresses(family, flags, NULL, pAdapterAddresses,
                                    &outBufLen);

    if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterAddresses);
        pAdapterAddresses = (IP_ADAPTER_ADDRESSES *)malloc(outBufLen);
        if (pAdapterAddresses == NULL) {
            printf("Memory allocation failed for IP_ADAPTER_ADDRESSES\n");
            return NULL;
        }
        dwRetVal = GetAdaptersAddresses(family, flags, NULL, pAdapterAddresses,
                                        &outBufLen);
    }

    if (dwRetVal != NO_ERROR) {
        printf("GetAdaptersAddresses failed with error: %ld\n", dwRetVal);
        free(pAdapterAddresses);
        return NULL;
    }

    return pAdapterAddresses;
}
