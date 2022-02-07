//
// Created by Pooya Khandel on 09/09/2021.
//

#include "logging.h"

Logging::Logging(int n_threads) {
    for (int i=0; i < n_threads; i++){
        llh_time[i] = 0;
        em_total_iteration_time[i] = 0;
        em_avg_iteration_time[i] = 0;
        em_computations_time[i] = 0;
        em_sync_time[i] = 0;
        em_updating_time[i] = 0;
        em_mem_time[i] = 0;
        this->n_threads = n_threads;
    }

}

void Logging::set_llh_time(int tid, double time) {
    llh_time[tid] = time;
}

void Logging::set_em_total_time(int tid, double time) {
    em_total_iteration_time[tid] = time;
}

void Logging::set_em_avg_time(int tid, double time) {
    em_avg_iteration_time[tid] = time;
}

void Logging::set_em_computations_time(int tid, double time) {
    em_computations_time[tid] = time;
}

double Logging::average_thread_time(std::map<int, double> thread_times) {
    return std::accumulate(std::begin(thread_times)
            , std::end(thread_times)
            , 0.0
            , [] (double value, const std::map<int, double>::value_type& p)
                           { return value + p.second; }
    ) / n_threads;
}

void Logging::set_em_sync_time(int tid, double time) {
    em_sync_time[tid] = time;
}

void Logging::set_em_updating_time(int tid, double time) {
    this->em_updating_time[tid] = time;
}

