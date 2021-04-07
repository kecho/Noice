#pragma once

#include "Utils.h"
#include <execution>

namespace ispc
{
    struct Image;
}

namespace noice
{

template <typename KernelT>
class KernelRunner
{
public:
    KernelRunner(int w, int h, int threadCount)
    : m_w(w), m_h(h)
    {
        int tileH = divUp<int>(h, threadCount);
        for (int tY = 0; tY < threadCount; ++tY)
        {
            m_jobContexts.emplace_back();
            JobContext& jc = m_jobContexts.back();
            jc.x0 = 0;
            jc.x1 = m_w;
            jc.y0 = tY * tileH;
            jc.y1 = std::min(jc.y0 + tileH, m_h);
        }
    }

    KernelT& kernel() { return m_kernel; }

    void run(ispc::Image& image)
    {
        if ((int)m_jobContexts.size() == 1)
        {
            m_kernel(0, 0, m_w, m_h, image);
            return;
        }

        std::for_each(std::execution::par, m_jobContexts.begin(), m_jobContexts.end(), [this, &image](JobContext& j) {
            m_kernel(j.x0, j.y0, j.x1, j.y1, image);
        });
    }

private:
    struct JobContext
    {
        int x0;
        int x1;
        int y0;
        int y1;
        int offset;
        int size;
    };

    std::vector<JobContext> m_jobContexts;
    int m_w;
    int m_h;
    KernelT m_kernel;
};

}
