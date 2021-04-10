#pragma once

namespace noice
{

template<typename T>
inline T divUp(T a, T b)
{
    return (a + b - 1) / b;
}


template<typename T>
inline unsigned int nextPowOf2(T v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return ++v;
}

template<typename T>
inline bool IsPowerOfTwo(T x)
{
    return (x & (x - 1)) == 0;
}

}
