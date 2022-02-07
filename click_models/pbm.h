//
// Created by Pooya Khandel on 09/09/2021.
//

#ifndef CASCADE_PARALLEL_PBM_H
#define CASCADE_PARALLEL_PBM_H

#include "base.h"


class PBM: public ClickModel{
public:
    PBM();
    PBM(PBM const &pbm);
    ~PBM() override = default;
    void say_hello() override;
    void test_change(int num) override;
    PBM* clone() override;
    void init_query_dependent_parameters(const Dataset& dataset, const std::vector<int>& train_tasks) override;
    Synchronizer* get_synchronizer(int n_threads) override;
    void get_shared_containers(std::vector<ParamContainer*>&, std::vector<ParamContainer*>&) override;
    void init_temporary_containers() override;
    void init_iteration_containers() override;
    void process_session(SearchSession) override;
    void update_thread_specific_parameters() override;
    void get_log_conditional_click_probs(SearchSession, std::vector<double>&) override;
    void get_full_click_probs(SearchSession, std::vector<double>&) override;
private:
    QueryDocumentParamContainer attr;
    QueryDocumentParamContainer base_attr;
    QueryDocumentParamContainer new_attr;
    RankedBasedContainer exams;
    RankedBasedContainer base_exams;
    RankedBasedContainer new_exams;
};


#endif //CASCADE_PARALLEL_PBM_H
