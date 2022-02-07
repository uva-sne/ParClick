//
// Created by Pooya Khandel on 01/11/2021.
//

#ifndef CASCADE_PARALLEL_DBN_H
#define CASCADE_PARALLEL_DBN_H

#include "base.h"
#include "factor.h"

class DBN: public ClickModel{
public:
    DBN() = default;
    DBN(DBN const &dbn);
    ~DBN() override = default;
    void say_hello() override;
    void test_change(int num) override;
    DBN* clone() override;
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

    QueryDocumentParamContainer sat;
    QueryDocumentParamContainer base_sat;
    QueryDocumentParamContainer new_sat;

    SingleParamContainer cont;
    SingleParamContainer base_cont;
    SingleParamContainer new_cont;

    std::array<double, 11> exam;
    std::array<double, 11> car;

    std::map<int, std::map<int, double>> click_probs;
    std::array<double, 11> exam_probs;

    int factor_inputs[8][3] = {{0, 0, 0},
                               {0, 0, 1},
                               {0, 1, 0},
                               {0, 1, 1},
                               {1, 0, 0},
                               {1, 0, 1},
                               {1, 1, 0},
                               {1, 1, 1},
    };
    std::array<double, 8> factor_values{{0}};
    double factor_sum{0};

    int last_click_rank;

    void compute_exam_car(SearchSession srs);
    void update_dbn_attr(SearchSession srs);
    void update_dbn_sat(SearchSession srs);
    void get_tail_clicks(SearchSession srs);
    void update_gamma(SearchSession srs);

};
#endif //CASCADE_PARALLEL_DBN_H
