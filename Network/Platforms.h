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
    #include <cstdio>
    #undef near
    #undef far
    #pragma comment(lib, "ws2_32.lib")

    // socket setup
    typedef SOCKET SocketType;
    #define ignore_not_reachable_error(sock) \
        do { \
            if ((sock) == INVALID_SOCKET) break; \
            const int __SIO_UDP_CONNRESET_CODE = _WSAIOW(IOC_VENDOR, 12); \
            BOOL __bNewBehavior = FALSE; \
            DWORD __dwBytesReturned = 0; \
            if (WSAIoctl((sock), __SIO_UDP_CONNRESET_CODE, &__bNewBehavior, sizeof(__bNewBehavior), NULL, 0, &__dwBytesReturned, NULL, NULL) == SOCKET_ERROR) { \
                printf("set socket failure\n"); \
                exit(EXIT_FAILURE); \
            } \
        } while (0)

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
    #define ignore_not_reachable_error(sock) (void)0

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