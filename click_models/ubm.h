//
// Created by Pooya Khandel on 01/11/2021.
//

#ifndef CASCADE_PARALLEL_UBM_H
#define CASCADE_PARALLEL_UBM_H
#include "base.h"


class UBM: public ClickModel{
public:
    UBM();
    UBM(UBM const &ubm);
    ~UBM() override = default;
    void say_hello() override;
    void test_change(int num) override;
    UBM* clone() override;
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

    RankPrevClickContainer exams;
    RankPrevClickContainer base_exams;
    RankPrevClickContainer new_exams;

};
#endif //CASCADE_PARALLEL_UBM_H
