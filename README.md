# SDLDisplay
Since gnuradio relies on SDL (version 1) the video sink of gnuradio does not work on macOS Mojave and Catalina.
This is a simple workaround to have a video sink working on macOS > 10.14. with SDL2.

## Usage:
First, create a named pipe:

`mkfifo /Users/<name>/stdout`

Compile and open SDLDisplay:

```
make
./SDLDisplay -i /Users/kesenheimer/stdout -h 600 -w 800
```

Use the file sink of gnuradio to write to the named pipe.
File sink properties:

```
File: /Users/<name>/stdout
Unbuffered: On
Append File: Append
```

Use `video_sdl_test.grc` as starting point.

## Requirements
```
sudo port install libsdl2 libsdl2_gfx libsdl2_image libsdl2_mixer
```
