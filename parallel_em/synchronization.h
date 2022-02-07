//
// Created by Pooya Khandel on 21/04/2021.
//

#ifndef CASCADE_PARALLEL_SYNCHRONIZATION_H
#define CASCADE_PARALLEL_SYNCHRONIZATION_H
#include <iostream>
#include <array>

#include "click_models/param.h"

class Synchronizer{
public:
    virtual ~Synchronizer() = default;
    virtual Synchronizer* clone() = 0;
    virtual void add_thread_params(std::vector<ParamContainer *>&, std::vector<ParamContainer*>&) = 0;
    virtual void say_hello() = 0;
    virtual void test_change(int thread_id) = 0;
    virtual void first_sync(int thread_id) = 0;
    virtual void second_sync(int thread_id) = 0;
};


class UnifiedSynchronizer: public Synchronizer{
protected:
    int n_threads{1};
    int max_rank_x_itr{10};
    int max_rank_y_itr{1};
    int n_containers{1};
    std::vector<std::array<std::array<double, 10>, 10>> numerator;
    std::vector<std::array<std::array<double, 10>, 10>> denominator;
    std::vector<std::vector<ParamContainer*>> shared_examinations_new;
    std::vector<std::vector<ParamContainer*>> shared_examinations_origin;
public:
    explicit UnifiedSynchronizer(int n_threads, int max_rank_x_itr,
                                 int max_rank_y_itr, int n_containers);
    ~UnifiedSynchronizer() override = default;
    UnifiedSynchronizer* clone() override;
    void add_thread_params(std::vector<ParamContainer *>&, std::vector<ParamContainer*>&) override;
    void say_hello() override;
    void test_change(int thread_id) override;
    void first_sync(int thread_id) override;
    void second_sync(int thread_id) override;
    virtual std::vector<std::vector<ParamContainer*>>& getter_shared_examinations_new();
};

class PBMSynchronizer: public UnifiedSynchronizer{
public:
    void second_sync(int thread_id) override;
    void first_sync(int thread_id) override;
    explicit PBMSynchronizer(int nThreads, int maxRankXItr, int maxRankYItr, int nContainers);
};

class CCMSynchronizer: public UnifiedSynchronizer{
public:
    void second_sync(int thread_id) override;
    void first_sync(int thread_id) override;
    explicit CCMSynchronizer(int nThreads, int maxRankXItr, int maxRankYItr, int nContainers);
};

class DBNSynchronizer: public UnifiedSynchronizer{
public:
    void second_sync(int thread_id) override;
    void first_sync(int thread_id) override;
    explicit DBNSynchronizer(int nThreads, int maxRankXItr, int maxRankYItr, int nContainers);
};

class UBMSynchronizer: public UnifiedSynchronizer{
public:
    void second_sync(int thread_id) override;
    void first_sync(int thread_id) override;
    explicit UBMSynchronizer(int nThreads, int maxRankXItr, int maxRankYItr, int nContainers);
};

#endif //CASCADE_PARALLEL_SYNCHRONIZATION_H
