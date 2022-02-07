//
// Created by Pooya Khandel on 09/09/2021.
//

#ifndef CASCADE_PARALLEL_CCM_H
#define CASCADE_PARALLEL_CCM_H

#include "base.h"
#include "factor.h"

class CCM: public ClickModel{
public:
    CCM()  = default;
    CCM(CCM const &ccm);
    ~CCM() override = default;
    void say_hello() override;
    void test_change(int num) override;
    CCM* clone() override;
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

    SingleParamContainer cont_no_click;
    SingleParamContainer base_cont_no_click;
    SingleParamContainer new_cont_no_click;

    SingleParamContainer cont_click_non_rel;
    SingleParamContainer base_cont_click_non_rel;
    SingleParamContainer new_cont_click_non_rel;

    SingleParamContainer cont_click_rel;
    SingleParamContainer base_cont_click_rel;
    SingleParamContainer new_cont_click_rel;

    std::array<double, 11> exam{};
    std::array<double, 11> car{};

    std::map<int, std::map<int, double>> click_probs;
    std::array<double, 11> exam_probs{};

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

    int last_click_rank{};
    void update_ccm_attr(SearchSession srs);
    void update_taus(SearchSession srs);
    void update_tau_1();
    void update_tau_2();
    void update_tau_3();
    void compute_exam_car(SearchSession srs);
    void get_tail_clicks(SearchSession srs);
};


#endif //CASCADE_PARALLEL_CCM_H
