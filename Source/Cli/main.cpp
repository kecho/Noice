#include <Noice/noice.h>
#include <iostream>

void main()
{
    int threadCounts[] = { 1, 2 ,4, 8, 16 };
    for (int& t : threadCounts)
    {
        std::cout << t << " thread: ";
        noice::makeBlueNoise(256, 256, t);
        std::cout << std::endl;
    }
}



