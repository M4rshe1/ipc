# Windows IP Configuration Tool (ipc)

This is a Windows IP configuration tool written in C, providing Linux-like `ip` command features. It displays network adapter information, including IP addresses, MAC addresses, DNS servers, and more.  It also supports DHCP lease renewal and release.

## Features

*   **Adapter Information:** Displays detailed information about network adapters, including name, description, MAC address, IP addresses (IPv4 and IPv6), DNS servers, and gateway addresses.
*   **Filtering:** Options to show all adapters (including disconnected ones), or filter by IPv4 or IPv6 addresses.
*   **Output Formatting:** Supports brief and detailed output formats.
*   **DHCP Control:** Allows renewing or releasing DHCP leases for specific adapters.
*   **Routing Table:** Displays the system's routing table.
*   **Network Statistics:** Shows network statistics.

## Usage

```
ipc [OPTIONS]
```

### Options

*   `-a`, `--all`: Show all adapters (including disconnected ones).
*   `-4`, `--ipv4`: Show only IPv4 addresses.
*   `-6`, `--ipv6`: Show only IPv6 addresses.
*   `-b`, `--brief`: Brief output format.
*   `-d`, `--details`: Show detailed information.
*   `-h`, `--help`: Display this help message.
*   `-n`, `--no-dns`: Don't show DNS information.
*   `-r`, `--route`: Show routing table.
*   `-s`, `--stats`: Show network statistics.
*   `--renew <adapter>`: Renew DHCP lease for the specified adapter.  Replace `<adapter>` with the adapter name (e.g., "Ethernet").
*   `--release <adapter>`: Release DHCP lease for the specified adapter. Replace `<adapter>` with the adapter name (e.g., "Ethernet").

### Examples

*   Show all adapters with detailed information:

    ```
    ipc -a -d
    ```

*   Show only IPv4 addresses in brief format:

    ```
    ipc -4 -b
    ```

*   Renew DHCP lease for the "Ethernet" adapter:

    ```
    ipc --renew "Ethernet"
    ```

*   Show the routing table:

    ```
    ipc -r
    ```

## Compilation

To compile this program, you will need a C compiler (like GCC) and the Windows SDK.  Here's how to compile it using GCC (MinGW) on Windows:

1.  **Install MinGW:** If you don't have it already, download and install MinGW (Minimalist GNU for Windows) from a reputable source.  Make sure to include `gcc`, `g++`, and `make` during the installation.  Also, ensure that the MinGW `bin` directory is added to your system's `PATH` environment variable.

2.  **Save the code:** Save the C code as `ipc.c`.

3.  **Compile:** Open a command prompt or MinGW shell and navigate to the directory where you saved `ipc.c`.  Then, run the following command:

    ```bash
    gcc ipc.c -o ipc -liphlpapi -lws2_32
    ```

    *   `gcc ipc.c`:  This tells GCC to compile the `ipc.c` source file.
    *   `-o ipc`: This specifies that the output executable file should be named `ipc.exe` (or just `ipc` in some environments).
    *   `-liphlpapi`: This links the `Iphlpapi.lib` library, which is required for accessing IP helper functions (like getting adapter information).
    *   `-lws2_32`: This links the `Ws2_32.lib` library, which is required for using Winsock (Windows Sockets) functions for network communication.

4.  **Run:** After successful compilation, you can run the program from the command prompt:

    ```bash
    ipc
    ```

## Dependencies

*   Windows SDK (for header files like `windows.h`, `winsock2.h`, `ws2tcpip.h`, `iphlpapi.h`)
*   MinGW (or another GCC distribution for Windows)

## Notes

*   This tool requires administrator privileges to renew or release DHCP leases.
*   The output format is designed to be similar to the Linux `ip` command, but it's not a complete replacement.
*   Error handling is basic; more robust error checking could be added.
*   The code uses `system()` calls for `route print` and `netstat -e`.  Consider using the Windows API directly for more control and security in a production environment.

