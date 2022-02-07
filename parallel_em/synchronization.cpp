//
// Created by Pooya Khandel on 21/04/2021.
//

#include "synchronization.h"

UnifiedSynchronizer::UnifiedSynchronizer(int n_threads, int max_rank_x_itr,
                                         int max_rank_y_itr, int n_containers) {
    this->n_threads = n_threads;
    this->max_rank_x_itr = max_rank_x_itr;
    this->max_rank_y_itr = max_rank_y_itr;
    this->n_containers = n_containers;
    this->numerator.reserve(n_threads);
    this->denominator.reserve(n_threads);
    this->shared_examinations_new.reserve(n_threads);
    this->shared_examinations_origin.reserve(n_threads);
}

UnifiedSynchronizer *UnifiedSynchronizer::clone() {
    return new UnifiedSynchronizer(*this);
}

void UnifiedSynchronizer::add_thread_params(std::vector<ParamContainer *> &shared_containers_new ,
                                            std::vector<ParamContainer *> &shared_containers_origin) {
    this->shared_examinations_new.push_back(shared_containers_new);
    this->shared_examinations_origin.push_back(shared_containers_origin);
}

void UnifiedSynchronizer::say_hello() {
}

void UnifiedSynchronizer::test_change(int thread_id) {

}

void UnifiedSynchronizer::first_sync(int thread_id) {
}

void UnifiedSynchronizer::second_sync(int thread_id) {

}

std::vector<std::vector<ParamContainer*>> &UnifiedSynchronizer::getter_shared_examinations_new() {
    return this->shared_examinations_new;
}

PBMSynchronizer::PBMSynchronizer(int nThreads, int maxRankXItr, int maxRankYItr, int nContainers)
        : UnifiedSynchronizer(nThreads, maxRankXItr, maxRankYItr, nContainers) {

}

void PBMSynchronizer::first_sync(int thread_id) {
    int ry_itr{0};
    for (int rx_itr = 0; rx_itr < 10; rx_itr++)
    {
        for (auto & shared_examination : this->shared_examinations_new)
        {
            this->numerator[thread_id][rx_itr][ry_itr] += shared_examination[0]->get_par(rx_itr, -1).numerator_val();
            this->denominator[thread_id][rx_itr][ry_itr] += shared_examination[0]->get_par(rx_itr, -1).denominator_val();
        }
        this->numerator[thread_id][rx_itr][ry_itr] -= this->n_threads - 1;
        this->denominator[thread_id][rx_itr][ry_itr] -= 2 * (this->n_threads - 1);
    }
}

void PBMSynchronizer::second_sync(int thread_id) {
    int ry_itr{0};
    for (int rx_itr=0; rx_itr < 10; rx_itr++){
        this->shared_examinations_origin[thread_id][0]->get_par(rx_itr, -1).set_values(
                this->numerator[thread_id][rx_itr][ry_itr],
                this->denominator[thread_id][rx_itr][ry_itr]);
    }
    this->numerator[thread_id] = { 0 };
    this->denominator[thread_id] = { 0 };
}


void CCMSynchronizer::second_sync(int thread_id) {
    int ry_itr{0};
    for (int single_exam_itr = 0; single_exam_itr < 3; single_exam_itr++)
    {
        this->shared_examinations_origin[thread_id][single_exam_itr]->get_par(
                0, -1).set_values(this->numerator[thread_id][single_exam_itr][ry_itr],
                                  this->denominator[thread_id][single_exam_itr][ry_itr]);
    }
    this->numerator[thread_id] = { 0 };
    this->denominator[thread_id] = { 0 };
}

void CCMSynchronizer::first_sync(int thread_id) {
    int ry_itr{0};
    for (int single_exam_itr = 0; single_exam_itr < 3; single_exam_itr++)
    {
        for (int thread_itr = 0; thread_itr < this->n_threads; thread_itr++)
        {
            this->numerator[thread_id][single_exam_itr][ry_itr] += this->shared_examinations_new[
                    thread_itr][single_exam_itr]->get_par(0, -1).numerator_val();
            this->denominator[thread_id][single_exam_itr][ry_itr] += this->shared_examinations_new[
                    thread_itr][single_exam_itr]->get_par(0, -1).denominator_val();
        }
        this->numerator[thread_id][single_exam_itr][ry_itr] -= this->n_threads - 1;
        this->denominator[thread_id][single_exam_itr][ry_itr] -= 2 * (this->n_threads - 1);
    }
}

CCMSynchronizer::CCMSynchronizer(int nThreads, int maxRankXItr, int maxRankYItr, int nContainers) : UnifiedSynchronizer(
        nThreads, maxRankXItr, maxRankYItr, nContainers) {
}

DBNSynchronizer::DBNSynchronizer(int nThreads, int maxRankXItr, int maxRankYItr, int nContainers) : UnifiedSynchronizer(
        nThreads, maxRankXItr, maxRankYItr, nContainers) {
}

void DBNSynchronizer::first_sync(int thread_id) {
//    UnifiedSynchronizer::first_sync(thread_id);
    int ry_itr{0};
    int single_exam_itr{0};
    for (int thread_itr = 0; thread_itr < this->n_threads; thread_itr++)
    {
        this->numerator[thread_id][single_exam_itr][ry_itr] += this->shared_examinations_new[
                thread_itr][single_exam_itr]->get_par(0, -1).numerator_val();
        this->denominator[thread_id][single_exam_itr][ry_itr] += this->shared_examinations_new[
                thread_itr][single_exam_itr]->get_par(0, -1).denominator_val();
    }
    this->numerator[thread_id][single_exam_itr][ry_itr] -= this->n_threads - 1;
    this->denominator[thread_id][single_exam_itr][ry_itr] -= 2 * (this->n_threads - 1);

}

void DBNSynchronizer::second_sync(int thread_id) {
//    UnifiedSynchronizer::second_sync(thread_id);
    int ry_itr{0};
    int single_exam_itr{0};
    this->shared_examinations_origin[thread_id][single_exam_itr]->get_par(
            0, -1).set_values(this->numerator[thread_id][single_exam_itr][ry_itr],
                              this->denominator[thread_id][single_exam_itr][ry_itr]);

    /*
    * CHECK IF BELOW LINES WORK CORRECTLY (SETTING AN 2D ARRAY ELEMENTS AS ZERO)
    */
    this->numerator[thread_id] = { 0 };
    this->denominator[thread_id] = { 0 };
}

void UBMSynchronizer::first_sync(int thread_id) {
    for (int rx_itr = 0; rx_itr < 10; rx_itr++)
    {
        for (int ry_itr = 0; ry_itr < 10; ry_itr++){
            for (auto & shared_examination : this->shared_examinations_new)
            {
                this->numerator[thread_id][rx_itr][ry_itr] += shared_examination[0]->get_par(rx_itr, ry_itr).numerator_val();
                this->denominator[thread_id][rx_itr][ry_itr] += shared_examination[0]->get_par(rx_itr, ry_itr).denominator_val();
            }
            this->numerator[thread_id][rx_itr][ry_itr] -= this->n_threads - 1;
            this->denominator[thread_id][rx_itr][ry_itr] -= 2 * (this->n_threads - 1);
        }
    }
}

void UBMSynchronizer::second_sync(int thread_id) {
    for (int rx_itr=0; rx_itr < 10; rx_itr++){
        for (int ry_itr=0; ry_itr < 10; ry_itr++){
            this->shared_examinations_origin[thread_id][0]->get_par(rx_itr, ry_itr).set_values(
                    this->numerator[thread_id][rx_itr][ry_itr],
                    this->denominator[thread_id][rx_itr][ry_itr]);
        }
    }
    this->numerator[thread_id] = { 0 };
    this->denominator[thread_id] = { 0 };
}

UBMSynchronizer::UBMSynchronizer(int nThreads, int maxRankXItr, int maxRankYItr, int nContainers) : UnifiedSynchronizer(
        nThreads, maxRankXItr, maxRankYItr, nContainers) {

}
