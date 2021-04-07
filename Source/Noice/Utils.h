#pragma once

namespace noice
{

template<typename T>
static inline T divUp(T a, T b)
{
    return (a + b - 1) / b;
}

}
