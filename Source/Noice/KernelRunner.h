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
    KernelRunner(int w, int h, int d, int threadCount)
    : m_w(w), m_h(h), m_d(d)
    {
        int tileH = divUp<int>(h, threadCount);
        int yOffset = 0;
        int heightLeft = h;
        for (int tY = 0; tY < threadCount && heightLeft > 0; ++tY)
        {
            int jobSzH = std::min(tileH, heightLeft);
            heightLeft -= jobSzH;

            m_jobContexts.emplace_back();
            JobContext& jc = m_jobContexts.back();
            jc.x0 = 0;
            jc.x1 = m_w;
            jc.y0 = yOffset;
            jc.y1 = jc.y0 + jobSzH;

            yOffset +=jobSzH;
        }
    }

    KernelT& kernel() { return m_kernel; }

    void run(ispc::Image& image)
    {
        for (int d = 0; d < m_d; ++d)
        {
            if ((int)m_jobContexts.size() == 1)
            {
                m_kernel(0, 0, m_w, m_h, d, image);
                continue;
            }

            std::for_each(std::execution::par, m_jobContexts.begin(), m_jobContexts.end(), [this, &d, &image](JobContext& j) {
                m_kernel(j.x0, j.y0, j.x1, j.y1, d, image);
            });
        }
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
    int m_d;
    KernelT m_kernel;
};

}
