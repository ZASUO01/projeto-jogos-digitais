//
// Created by pedro-souza on 23/11/2025.
//
#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS 1
#elif defined(__APPLE__)
    #define PLATFORM_MACOS 1
#elif defined(__linux__)
    #define PLATFORM_LINUX 1
#endif

#ifdef PLATFORM_WINDOWS
    #undef main

    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #pragma comment(lib, "ws2_32.lib")

    // disable not reachable error messages
    #define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)
    BOOL bNewBehavior = FALSE;
    DWORD dwBytesReturned = 0;
    WSAIoctl(mySock, SIO_UDP_CONNRESET, &bNewBehavior, sizeof(bNewBehavior), NULL, 0, &dwBytesReturned, NULL, NULL)

    typedef SOCKET SocketType;

    #define POLL_FD_TYPE WSAPOLLFD
    #define socket_poll WSAPoll
    #define create_socket(domain, type, protocol) (static_cast<int>(socket(domain, type, protocol)))
    #define close_socket(sock) closesocket((SOCKET)(sock))

    #define socket_sendto(socket, buffer, length, flags, dest_addr, addrlen) \
        sendto(static_cast<SOCKET>(socket), reinterpret_cast<const char*>(buffer), static_cast<int>(length), flags, \
        dest_addr, static_cast<int>(addrlen))

    #define socket_recvfrom(socket, buffer, length, flags, src_addr, addrlen) \
        recvfrom(static_cast<SOCKET>(socket), reinterpret_cast<char*>(buffer), \
        static_cast<int>(length), flags, src_addr, static_cast<int*>(addrlen))

    #define socket_bind(socket, addr, addrlen) \
        bind(static_cast<SOCKET>(socket), addr, static_cast<int>(addrlen))

    #define networkingInit() do { \
        WSADATA wsaData; \
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData); \
        if (result != 0) { \
            printf("Winsock init failure: %d\n", result); \
            exit(EXIT_FAILURE); \
        } \
        printf("Winsock initialized\n"); \
    } while(0)

    #define networkingCleanup() do { \
        WSACleanup(); \
        printf("Winsock finished\n"); \
    } while (0)

#elif defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
    #include <sys/socket.h>
    #include <sys/select.h>
    #include <sys/time.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <poll.h>

    typedef int SocketType;
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR (-1)

    #define POLL_FD_TYPE struct pollfd
    #define socket_poll poll
    #define close_socket close

    #define POLL_FD_TYPE struct pollfd
    #define socket_poll poll
    #define create_socket(domain, type, protocol) socket(domain, type, protocol)
    #define close_socket close

    #define socket_sendto(socket, buffer, length, flags, dest_addr, addrlen) \
        sendto(socket, buffer, length, flags, dest_addr, addrlen)

    #define socket_recvfrom(socket, buffer, length, flags, src_addr, addrlen) \
        recvfrom(socket, buffer, length, flags, src_addr, addrlen)

    #define socket_bind(socket, addr, addrlen) bind(socket, addr, addrlen)

    #define networkingInit()
    #define networkingCleanup()
#endif

