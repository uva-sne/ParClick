//
// Created by Pooya Khandel on 15/04/2021.
//

#ifndef CASCADE_PARALLEL_PARALLEL_EM_H
#define CASCADE_PARALLEL_PARALLEL_EM_H
#include <iostream>
#include <string>
#include "click_models/param.h"
#include "barrier.h"
#include "utils/logging.h"
#include "click_models/base.h"
#include "click_models/evaluation.h"

void em_thread(int which_thread, const int& n_itr, Logging& logger,
               ClickModel* cm,
               Synchronizer*,
               Dataset& pr_dataset,
               ThreadPartition& tasks,
               std::map<int, std::array<double, 2>>& llh_task,
               std::map<int, Perplexity>& ppl_task,
               Barrier& br1, Barrier& br2
);

void em_parallel(ParallelClickModel& pcm, Dataset& pr_dataset,
                 Partition** pr_part, int n_threads, int n_itr);


#endif //CASCADE_PARALLEL_PARALLEL_EM_H
