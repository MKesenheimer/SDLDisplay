#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include <SDL_net.h>

main(int argc, char *argv[]) {
    IPaddress serverIP;
    UDPsocket udpsock;
    UDPpacket* recv_packet;
    SDLNet_SocketSet socketset = NULL;
    int numused;
    static const int MAX_PACKET_SIZE = 512;

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
    if ( socketset == NULL ) {
      fprintf(stderr, "Couldn't create socket set: %s\n", SDLNet_GetError());
      exit(2);
    }

    numused = SDLNet_UDP_AddSocket(socketset,udpsock);
    if(numused==-1) {
      printf("SDLNet_AddSocket: %s\n", SDLNet_GetError());
      exit(2);
    }

    recv_packet = SDLNet_AllocPacket(MAX_PACKET_SIZE);
    if(!recv_packet) {
      printf("Could not allocate packet\n");
      exit(2);
    }

    while(1) {
      SDLNet_CheckSockets(socketset, ~0);
      if (SDLNet_SocketReady(udpsock)) {
        if (SDLNet_UDP_Recv(udpsock, recv_packet)) {
          SDLNet_UDP_Send(udpsock, -1, recv_packet);

          //format log
          int len = recv_packet->len;
          char temp[MAX_PACKET_SIZE+2];
          memcpy(temp, recv_packet->data, len);

          if (temp[len-1] == '\n') {
           temp[len-1] = '\\';
           temp[len] = 'n';
           temp[len+1] = '\0';
          } else {
           temp[len] = '\0';
          }

          char hoststring[128];
          inet_ntop(AF_INET,&recv_packet->address.host,hoststring,128),

          printf("got: \"%s\" from: %s:%d\n",
              temp,
              hoststring,
              ntohs(recv_packet->address.port));
        }
      }
    }
    return 0;
}
