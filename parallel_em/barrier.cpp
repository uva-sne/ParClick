//
// Created by Pooya Khandel on 06/01/2021.
//

#include "barrier.h"


Barrier::Barrier(std::size_t count) : m_count{ count }, m_initial{ count }, m_state{ State::Down } { }

/// Blocks until all N threads reach here
void Barrier::Sync()
{
    std::unique_lock<std::mutex> lock{ m_mutex };

    if (m_state == State::Down)
    {
        // Counting down the number of syncing threads
        if (--m_count == 0) {
            m_state = State::Up;
            m_cv.notify_all();
        }
        else {
            m_cv.wait(lock, [this] { return m_state == State::Up; });
        }
    }

    else // (m_state == State::Up)
    {
        // Counting back up for Auto reset
        if (++m_count == m_initial) {
            m_state = State::Down;
            m_cv.notify_all();
        }
        else {
            m_cv.wait(lock, [this] { return m_state == State::Down; });
        }
    }
}
