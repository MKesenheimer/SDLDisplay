// compiling:
// gcc udp_echo.c `sdl2-config --cflags --static-libs` -lSDL2_net -o udp_echo
// send udp packet with $ nc -4u localhost 3333
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <SDL_net.h>

int getUDPPacket(const SDLNet_SocketSet* const socketset, const UDPsocket* const udpsock, 
    UDPpacket* const recv_packet, char* const chars) {
    SDLNet_CheckSockets(*socketset, ~0);
    if (SDLNet_SocketReady(*udpsock)) {
        if (SDLNet_UDP_Recv(*udpsock, recv_packet)) {
            //SDLNet_UDP_Send(*udpsock, -1, recv_packet);
            int len = recv_packet->len;
            memcpy(chars, recv_packet->data, len);
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    //IPaddress serverIP;
    UDPsocket udpsock;
    UDPpacket* recv_packet;
    SDLNet_SocketSet socketset = NULL;
    int numused;
    static const int MAX_PACKET_SIZE = 1;

    if (SDLNet_Init() < 0) {
        printf("Couldn't initialize net: %s\n", SDLNet_GetError());
        exit(1);
    }

    udpsock = SDLNet_UDP_Open(3333);
    if(!udpsock) {
        printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        exit(2);
    } else {
        printf("listening on 0.0.0.0:3333\n");
    }

    socketset = SDLNet_AllocSocketSet(2);
    if (socketset == NULL) {
        fprintf(stderr, "Couldn't create socket set: %s\n", SDLNet_GetError());
        exit(2);
    }

    numused = SDLNet_UDP_AddSocket(socketset, udpsock);
    if(numused == -1) {
        printf("SDLNet_AddSocket: %s\n", SDLNet_GetError());
        exit(2);
    }

    recv_packet = SDLNet_AllocPacket(MAX_PACKET_SIZE);
    if(!recv_packet) {
        printf("Could not allocate packet\n");
        exit(2);
    }

    char chars[MAX_PACKET_SIZE];
    while(1) {
        getUDPPacket(&socketset, &udpsock, recv_packet, chars);
        for (int i = 0; i < MAX_PACKET_SIZE; ++i) {
            if(chars[i] != '\0') {
                int a = (chars[i] & 0xff) - 1;
                //int a = atoi(chars[i]);
                printf("%d\n", a);
            }
        }
    }
    return 0;
}