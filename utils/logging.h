//
// Created by Pooya Khandel on 09/09/2021.
//

#ifndef CASCADE_PARALLEL_LOGGING_H
#define CASCADE_PARALLEL_LOGGING_H

#include <iostream>
#include <numeric>
#include <array>
#include <map>



class Logging{
private:
    double n_threads;
public:
    std::map<int, double> llh_time;
    std::map<int, double> em_total_iteration_time;
    std::map<int, double> em_avg_iteration_time;
    std::map<int, double> em_computations_time;
    std::map<int, double> em_sync_time;
    std::map<int, double> em_mem_time;
    std::map<int, double> em_updating_time;
    explicit Logging(int n_threads);
    void set_llh_time(int tid, double time);
    void set_em_total_time(int tid, double time);
    void set_em_avg_time(int tid, double time);
    void set_em_computations_time(int tid, double time);
    void set_em_sync_time(int tid, double time);
    double average_thread_time(std::map<int, double> thread_times);
    void set_em_updating_time(int tid, double time);
};



#endif //CASCADE_PARALLEL_LOGGING_H
