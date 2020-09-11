#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <time.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <complex>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL_render.h>
#include <SDL_net.h>

#include "SDLAuxiliary.h"
#include "Timer.h"
#include "CommandLineParser.h"

#include <fstream>
#include <filesystem>

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

int main(int argc, char* args[])
{
    // Parse the command line arguments
    std::string set("Mandelbrot");
    if (CommandLineParser::cmdOptionExists(args, args + argc, "-s"))
        set = static_cast<std::string>(CommandLineParser::getCmdOption(args, args + argc, "-s"));

    int SCREEN_WIDTH = 400;
    std::string width;
    if (CommandLineParser::cmdOptionExists(args, args + argc, "-w"))
        SCREEN_WIDTH = std::stoi(static_cast<std::string>(CommandLineParser::getCmdOption(args, args + argc, "-w")));

    int SCREEN_HEIGHT = 300;
    if (CommandLineParser::cmdOptionExists(args, args + argc, "-h"))
        SCREEN_HEIGHT = std::stoi(static_cast<std::string>(CommandLineParser::getCmdOption(args, args + argc, "-h")));

    int FRAMES_PER_SECOND = 20; //Fps auf 20 festlegen
    if (CommandLineParser::cmdOptionExists(args, args + argc, "-f"))
        FRAMES_PER_SECOND = std::stoi(static_cast<std::string>(CommandLineParser::getCmdOption(args, args + argc, "-f")));


    //Timer zum festlegen der FPS
    Timer fps;
    //Timer zum errechnen der weltweit vergangenen Zeit
    Timer worldtime;
    worldtime.start();
    int frame = 0; //take records of frame number
    bool cap = false; //Framecap an oder ausschalten

    // SDL stuff
    //Setup our window and renderer, this time let's put our window in the center of the screen
    SDL_Window *window = SDL_CreateWindow("Video Display", SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        SDLAuxiliary::logSDLError(std::cout, std::string("Couldn't create window ") + std::string(SDLNet_GetError()));
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL)
    {
        SDLAuxiliary::logSDLError(std::cout, std::string("Couldn't create renderer ") + std::string(SDLNet_GetError()));
        SDLAuxiliary::cleanup(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (texture == NULL)
    {
        SDLAuxiliary::logSDLError(std::cout, std::string("CreateTexture ") + std::string(SDLNet_GetError()));
        SDLAuxiliary::cleanup(renderer, window);
        SDL_Quit();
        return 1;
    }

    // UDP stuff
    //IPaddress serverIP;
    UDPsocket udpsock;
    UDPpacket* recv_packet;
    SDLNet_SocketSet socketset = NULL;
    int numused;
    static const int MAX_PACKET_SIZE = 1024;
    udpsock = SDLNet_UDP_Open(3333);
    if(!udpsock) {
        SDLAuxiliary::logSDLError(std::cout, std::string("Couldn't initialize net: ") + std::string(SDLNet_GetError()));
        SDLAuxiliary::cleanup(renderer, window);
        SDL_Quit();
    } else {
        printf("listening on 0.0.0.0:3333\n");
    }

    socketset = SDLNet_AllocSocketSet(2);
    if (socketset == NULL) {
        SDLAuxiliary::logSDLError(std::cout, std::string("Couldn't create socket set: ") + std::string(SDLNet_GetError()));
        SDLAuxiliary::cleanup(renderer, window);
        SDL_Quit();
    }

    numused = SDLNet_UDP_AddSocket(socketset, udpsock);
    if(numused == -1) {
        SDLAuxiliary::logSDLError(std::cout, std::string("SDLNet_AddSocket: ") + std::string(SDLNet_GetError()));
        SDLAuxiliary::cleanup(renderer, window);
        SDL_Quit();
    }

    recv_packet = SDLNet_AllocPacket(MAX_PACKET_SIZE);
    if(!recv_packet) {
        SDLAuxiliary::logSDLError(std::cout, std::string("Could not allocate packet."));
        SDLAuxiliary::cleanup(renderer, window);
        SDL_Quit();
    }

    // variables for the file sink
    //unsigned short number = 0;
    const int numberOfPixels = SCREEN_WIDTH * SCREEN_HEIGHT;
    uint8_t ypixels[numberOfPixels];
    uint8_t upixels[numberOfPixels / 4];
    uint8_t vpixels[numberOfPixels / 4];
    for (int i = 0; i < numberOfPixels / 4; ++i) {
        upixels[i] = 128;
        vpixels[i] = 128;
    }
    //const size_t size = sizeof(number);
    char chars[MAX_PACKET_SIZE];
    bool quit = false;
    SDL_Event e;
    int j = 0;
    while (!quit)
    {
        // start the fps timer
        fps.start();

        // handle events
        while (SDL_PollEvent(&e))
            if (e.type == SDL_QUIT)
                quit = true;

        getUDPPacket(&socketset, &udpsock, recv_packet, chars);
        for (int i = 0; i < MAX_PACKET_SIZE; ++i) {
            if(chars[i] != '\0') {
                int number = (chars[i] & 0xff) - 1;
                ypixels[j++] = number;
                if (j >= numberOfPixels) j = 0;
                //printf("%d\n", number);
            }
        }

        // display image
        //for (int i = 0; i < numberOfPixels; ++i)
        //    pixels[i] = rand() % 255;  
        SDL_UpdateYUVTexture(texture, NULL, ypixels, SCREEN_WIDTH, upixels, SCREEN_WIDTH / 2, vpixels, SCREEN_WIDTH / 2);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        //increment the frame number
        frame++;
        //apply the fps cap
        if((cap == true) && (fps.getTicks() < 1000/FRAMES_PER_SECOND)) {
            SDL_Delay( (1000/FRAMES_PER_SECOND) - fps.getTicks() );
        }

        //update the window caption
        if(worldtime.getTicks() > 1000) {
            std::stringstream caption;
            caption << "Video Display, FPS = " << 1000.f*frame/worldtime.getTicks();
            SDL_SetWindowTitle(window,caption.str().c_str());
            worldtime.start();
            frame = 0;
        }
    }

    //Destroy the various items
    SDLAuxiliary::cleanup(texture, renderer, window );
    IMG_Quit();
    SDL_Quit();

    return 0;
}
