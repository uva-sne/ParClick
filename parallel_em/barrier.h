//
// Created by Pooya Khandel on 06/01/2021.
//

#ifndef PBMCPP_PBM_H
#define PBMCPP_PBM_H
#include <vector>
#include <mutex>
#include <thread>
#include <array>
#include <algorithm>
#include <condition_variable>

#include "data/search.h"
#include "click_models/param.h"
#include "data/dataset.h"


class Barrier
{
private:
    std::mutex m_mutex;
    std::condition_variable m_cv;

    size_t m_count;
    const size_t m_initial;

    enum State : unsigned char {
        Up, Down
    };
    State m_state;

public:
    explicit Barrier(std::size_t count);
    /// Blocks until all N threads reach here
    void Sync();
};

#endif //PBMCPP_PBM_H
