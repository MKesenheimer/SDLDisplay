// Usage:
// create a named pipe:
// $ mkfifo /Users/<name>/stdout
// open SDL-Display:
// $ ./SDLDisplay -i /Users/kesenheimer/stdout -h 600 -w 800
// use the file sink of gnu-radio to write to the named pipe:
// File Sink:
// File: /Users/<name>/stdout
// Unbuffered: On
// Append File: Append

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

#include "SDLTools/Utilities.h"
#include "SDLTools/Timer.h"
#include "SDLTools/CommandLineParser.h"

#include <fstream>
#include <filesystem>

// TODO: Usage mit --help ausgeben
// TODO: Anzahl der Kommandozeilenargumente prüfen

// TODO: automatisieren
// read input from a file dump or a named pipe
//#define NORMAL_FILE
#define NAMED_PIPE

int main(int argc, char* args[])
{
    // Parse the command line arguments
    std::string set("SDLDisplay");
    if (sdl::auxiliary::CommandLineParser::cmdOptionExists(args, args + argc, "-s"))
        set = static_cast<std::string>(sdl::auxiliary::CommandLineParser::getCmdOption(args, args + argc, "-s"));

    int SCREEN_WIDTH = 800;
    std::string width;
    if (sdl::auxiliary::CommandLineParser::cmdOptionExists(args, args + argc, "-w"))
        SCREEN_WIDTH = std::stoi(static_cast<std::string>(sdl::auxiliary::CommandLineParser::getCmdOption(args, args + argc, "-w")));

    int SCREEN_HEIGHT = 600;
    if (sdl::auxiliary::CommandLineParser::cmdOptionExists(args, args + argc, "-h"))
        SCREEN_HEIGHT = std::stoi(static_cast<std::string>(sdl::auxiliary::CommandLineParser::getCmdOption(args, args + argc, "-h")));

    int FRAMES_PER_SECOND = 60;
    if (sdl::auxiliary::CommandLineParser::cmdOptionExists(args, args + argc, "-f"))
        FRAMES_PER_SECOND = std::stoi(static_cast<std::string>(sdl::auxiliary::CommandLineParser::getCmdOption(args, args + argc, "-f")));

    std::string filename("dump.bin");
    if (sdl::auxiliary::CommandLineParser::cmdOptionExists(args, args + argc, "-i"))
        filename = static_cast<std::string>(sdl::auxiliary::CommandLineParser::getCmdOption(args, args + argc, "-i"));

    std::cout << "SCREEN_WIDTH = " << SCREEN_WIDTH << ", SCREEN_HEIGHT = " << SCREEN_HEIGHT << std::endl;

    // Timer zum festlegen der FPS
    sdl::auxiliary::Timer fps;
    // Timer zum errechnen der weltweit vergangenen Zeit
    sdl::auxiliary::Timer worldtime;
    worldtime.start();
    int frame = 0; // take records of frame number
    bool cap = true; // Framecap an oder ausschalten

    // SDL stuff
    // Setup our window and renderer, this time let's put our window in the center of the screen
    SDL_Window *window = SDL_CreateWindow("Video Display", SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        sdl::auxiliary::Utilities::logSDLError(std::cout, "CreateWindow");
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        sdl::auxiliary::Utilities::logSDLError(std::cout, "CreateRenderer");
        sdl::auxiliary::Utilities::cleanup(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (texture == NULL) {
        sdl::auxiliary::Utilities::logSDLError(std::cout, "CreateTexture");
        sdl::auxiliary::Utilities::cleanup(renderer, window);
        SDL_Quit();
        return 1;
    }

    // variables for the file sink
    unsigned short number = 0;
    const int numberOfPixels = SCREEN_WIDTH * SCREEN_HEIGHT;
    uint8_t ypixels[numberOfPixels];
    uint8_t upixels[numberOfPixels / 4];
    uint8_t vpixels[numberOfPixels / 4];
    for (int i = 0; i < numberOfPixels / 4; ++i) {
        upixels[i] = 128;
        vpixels[i] = 128;
    }
    const size_t size = sizeof(number);
    std::ifstream filesink(filename, std::ios::in | std::ios::binary);

#ifdef NORMAL_FILE
    unsigned long fpointer = 0;
    unsigned long flength = 0;
#endif

    bool quit = false;
    SDL_Event e;
    while (!quit)
    {
        // start the fps timer
        fps.start();

        // handle events
        while (SDL_PollEvent(&e))
            if (e.type == SDL_QUIT)
                quit = true;
        
#ifdef NORMAL_FILE
        // read the file size
        filesink.seekg(0, filesink.end);
        flength = filesink.tellg() / size;

        // back to old position and read new image data
        if (fpointer < flength - numberOfPixels)
        {
            //std::cout << "fpointer   = " << fpointer << std::endl;
            //std::cout << "flength    = " << flength << std::endl;
            filesink.seekg(fpointer, filesink.beg);
            for (int i = 0; i < numberOfPixels; ++i) {
                filesink.read(reinterpret_cast<char*>(&number), size);
                ypixels[i] = number;
                fpointer += size;
                std::cout << number << std::endl;
            }
        }
#endif

#ifdef NAMED_PIPE
        for (int i = 0; i < numberOfPixels; ++i) {
            filesink.read(reinterpret_cast<char*>(&number), size);
            ypixels[i] = number;
            //std::cout << number << std::endl;
        }
#endif

        // display image
        SDL_UpdateYUVTexture(texture, NULL, ypixels, SCREEN_WIDTH, upixels, SCREEN_WIDTH / 2, vpixels, SCREEN_WIDTH / 2);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        // increment the frame number
        frame++;
        // apply the fps cap
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
    filesink.close();
    sdl::auxiliary::Utilities::cleanup(texture, renderer, window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
