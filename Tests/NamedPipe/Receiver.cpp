// mkfifo stdout
// g++ Sender.cpp
// ./a.out
// echo -n a > stdout

#include <iostream>
#include <fstream>

int main(int argc, char* args[])
{
    // variables for the file sink
    unsigned short number = 0;
    std::string filename("stdout");
    const size_t size = sizeof(number);
    std::ifstream filesink(filename, std::ios::in | std::ios::binary);

    for (int i = 0; i < 1; ++i) {
        filesink.read(reinterpret_cast<char*>(&number), size);
        std::cout << number << std::endl;
    }
}