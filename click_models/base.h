//#include <__bit_reference>
//
// Created by Pooya Khandel on 15/04/2021.
//

#ifndef CASCADE_PARALLEL_BASE_H
#define CASCADE_PARALLEL_BASE_H
#include "param.h"
#include "parallel_em/synchronization.h"


class ClickModel{
public:
    virtual void init_query_dependent_parameters(const Dataset& dataset, const std::vector<int>& train_tasks) = 0;
    virtual ClickModel* clone() = 0;
    virtual void say_hello() = 0;
    virtual void test_change(int num) = 0;
    virtual ~ClickModel() = default;
    virtual Synchronizer* get_synchronizer(int n_threads) = 0;
    virtual void get_shared_containers(std::vector<ParamContainer*>&, std::vector<ParamContainer*>&) = 0;
    virtual void init_temporary_containers() = 0;
    virtual void init_iteration_containers() = 0;
    virtual void process_session(SearchSession) = 0;
    virtual void update_thread_specific_parameters() = 0;
    virtual void get_log_conditional_click_probs(SearchSession, std::vector<double>&) = 0;
    virtual void get_full_click_probs(SearchSession, std::vector<double>&) = 0;
};


ClickModel* create_cm(int model_type);

class ParallelClickModel{
private:
    ClickModel* original_base_model;
    std::vector<ClickModel*> thread_models;
    int num_threads{1};
    Synchronizer* synchronizer;
public:
    ParallelClickModel(ClickModel* cm, int num_threads);
    ~ParallelClickModel();
    void init_query_dependent_parameters(const Dataset& dataset, Partition** partition);
    ClickModel* get_thread_click_model(int thread_id);
    Synchronizer* fetch_synchronizer();
};


#endif //CASCADE_PARALLEL_BASE_H
