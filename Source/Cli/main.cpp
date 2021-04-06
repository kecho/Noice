#include <Noice/noice.h>
#include <iostream>

void main()
{
    int threadCounts[] = { 16 };
    //int threadCounts[] = { 16 };
    for (int& t : threadCounts)
    {
        std::cout << t << " thread: ";
        noice::makeBlueNoise(512, 512, t);
        std::cout << std::endl;
    }
}



