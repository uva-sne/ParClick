//
// Created by Pooya Khandel on 01/11/2021.
//

#include "dbn.h"

DBN::DBN(const DBN &dbn) {

}

void DBN::say_hello() {
    std::cout << "DBN says hello!" << std::endl;
}

void DBN::test_change(int num) {

}

DBN *DBN::clone() {
    return new DBN(*this);
}

void DBN::init_query_dependent_parameters(const Dataset &dataset, const std::vector<int> &train_tasks) {
    this->attr.initialize_for_threads(dataset, train_tasks);
    this->sat.initialize_for_threads(dataset, train_tasks);
}

Synchronizer *DBN::get_synchronizer(int n_threads) {
    Synchronizer* sync = new DBNSynchronizer(n_threads, 1, 1, 1);
    return sync;
}

void DBN::get_shared_containers(std::vector<ParamContainer*>& temp_container_new,
                                std::vector<ParamContainer*>& temp_container_origin) {
    temp_container_new.push_back(&this->new_cont);
    temp_container_origin.push_back(&this->cont);
}

void DBN::init_temporary_containers() {
    this->base_attr = this->attr;
    this->base_sat = this->sat;
    this->base_cont = this->cont;
}

void DBN::init_iteration_containers() {
    this->new_attr = this->base_attr;
    this->new_sat = this->base_sat;
    this->new_cont = this->base_cont;
}

void DBN::update_thread_specific_parameters() {
    this->attr = this->new_attr;
    this->sat = this->new_sat;
}

void DBN::get_log_conditional_click_probs(SearchSession search_ses, std::vector<double>& log_click_probs) {
    std::vector<SearchResult> results = search_ses.get_sr();
    SearchResult temp_sr;
    double attr_val, sat_val, gamma_val;
    double exam_val, click_prob;

    exam_val = 1.0;

    for (int res_itr=0; res_itr < 10; res_itr++){

        temp_sr = results[res_itr];
        attr_val = this->attr.get_par(temp_sr.get_query(), temp_sr.get_doc_id()).value();
        sat_val = this->sat.get_par(temp_sr.get_query(), temp_sr.get_doc_id()).value();
        gamma_val = this->cont.get_par(0, -1).value();

        if (temp_sr.get_click() == 1){
            click_prob = attr_val * exam_val;
            exam_val = gamma_val * ( 1- sat_val);
        } else{
            click_prob = 1 - attr_val * exam_val;
            exam_val *= gamma_val * ( 1 - attr_val) / click_prob;
        }

        log_click_probs.push_back(std::log(click_prob));
    }


}

void DBN::get_full_click_probs(SearchSession search_ses, std::vector<double> &full_click_probs) {
    std::vector<SearchResult> sr_vec = search_ses.get_sr();
    double attr_val, sat_val, gamma_val;
    double ex_value, click_prob, atr_mul_ex;

    ex_value = 1.0;

    for (int rank=0; rank < 10; rank++) {
        SearchResult session = sr_vec[rank];
        attr_val = this->attr.get_par(session.get_query(), session.get_doc_id()).value();
        sat_val = this->sat.get_par(session.get_query(), session.get_doc_id()).value();
        gamma_val = this->cont.get_par(0, -1).value();

        atr_mul_ex = attr_val * ex_value;

        int click{session.get_click()};
        if (click == 1) {
            full_click_probs.push_back(atr_mul_ex);
        }
        else {
            full_click_probs.push_back(1 - atr_mul_ex);
        }

        ex_value *= gamma_val * (1 - attr_val) + gamma_val * attr_val * (1 - sat_val);
    }
}

void DBN::process_session(SearchSession search_ses) {
    this->last_click_rank = search_ses.get_last_click_rank();
    this->compute_exam_car(search_ses);
    this->update_dbn_attr(search_ses);
    this->update_dbn_sat(search_ses);
    this->get_tail_clicks(search_ses);
    this->update_gamma(search_ses);
}

void DBN::compute_exam_car(SearchSession srs) {
    this->exam[0] = 1;
    int exm_itr{0};
    int qid, did;
    double attr_val, sat_value, gamma_value, ex_value;

    double temp, car_val;

    std::vector<SearchResult> srs_vec = srs.get_sr();

    std::vector<std::array<double, 2>> car_helper;
    car_helper.reserve(10);

    for (auto sr: srs_vec){
        qid = sr.get_query();
        did = sr.get_doc_id();

        attr_val = this->attr.get_par(qid, did).value();
        sat_value = this->sat.get_par(qid, did).value();
        gamma_value = this->cont.get_par(0, -1).value();
        ex_value = this->exam[exm_itr];

        temp = gamma_value * (1 - attr_val);
        ex_value *= temp + gamma_value * attr_val * (1 - sat_value);

        car_helper[exm_itr][0] = attr_val;
        car_helper[exm_itr][1] = temp;

        exm_itr += 1;
        this->exam[exm_itr] = ex_value;

    }

    this->car = {0};
    for (int car_itr=9; car_itr > -1; car_itr--){
        car_val = this->car[car_itr + 1];
        this->car[car_itr] = car_helper[car_itr][0] + car_helper[car_itr][1] * car_val;
    }

}

void DBN::update_dbn_attr(SearchSession srs) {
    std::vector<SearchResult> s_results = srs.get_sr();
    double numerator_update, denominator_update;

    int qid, did, click, rank;
    double attr_val, exam_val, car_val;

    for (auto sr: s_results){
        numerator_update = 0;
        denominator_update = 1;

        click = sr.get_click();
        rank = sr.get_doc_rank();
        qid = sr.get_query();
        did = sr.get_doc_id();

        if (click == 1){
            numerator_update += 1;
        }
        else if (rank >= this->last_click_rank){
            attr_val = this->attr.get_par(qid, did).value();
            exam_val = this->exam[rank];
            car_val = this->car[rank];

            numerator_update += (attr_val * (1 - exam_val)) / (1 - exam_val * car_val);
        }
        this->new_attr.get_par(qid, did).add_to_values(numerator_update, denominator_update);
    }
}

void DBN::update_dbn_sat(SearchSession srs) {
    std::vector<SearchResult> s_results = srs.get_sr();
    double numerator_update, denominator_update;

    int qid, did, click, rank;
    double gamma_val, car_val, sat_val;
    for (auto sr: s_results){
        click = sr.get_click();

        if (click == 1){
            numerator_update = 0.0;
            denominator_update = 1.0;

            qid = sr.get_query();
            did = sr.get_doc_id();
            rank = sr.get_doc_rank();
            if (rank == this->last_click_rank){
                sat_val = this->sat.get_par(qid, did).value();
                gamma_val = this->cont.get_par(0, -1).value();
                if (rank < s_results.size() - 1){
                    car_val = this->car[rank + 1];
                } else{
                    car_val = 0;
                }

                numerator_update += sat_val / (1 - (1 - sat_val) * gamma_val * car_val);
            }
            this->new_sat.get_par(qid, did).add_to_values(numerator_update, denominator_update);
        }
    }

}

void DBN::get_tail_clicks(SearchSession srs) {
    std::vector<SearchResult> results = srs.get_sr();
    this->exam_probs[0] = 1.0;
    SearchResult temp_sr;
    double gamma_val, exam_val, click_prob;

    for (int start_rank = 0; start_rank < 10; start_rank++){
        exam_val = 1.0;
        int ses_itr{0};
        for (int res_itr = start_rank; res_itr < 10; res_itr++){
            temp_sr = results[ses_itr];
            double attr_val = this->attr.get_par(temp_sr.get_query(), temp_sr.get_doc_id()).value();
            double sat_val = this->sat.get_par(temp_sr.get_query(), temp_sr.get_doc_id()).value();
            gamma_val = this->cont.get_par(0, -1).value();

            if (results[res_itr].get_click() == 1){
                click_prob = attr_val * exam_val;
                exam_val = gamma_val * ( 1- sat_val);
            }
            else{
                click_prob = 1 - attr_val * exam_val;
                exam_val *= gamma_val * ( 1 - attr_val) / click_prob;
            }

            this->click_probs[start_rank][ses_itr] = click_prob;
            if (start_rank == 0){
                this->exam_probs[ses_itr+1] = exam_val;
            }
            ses_itr++;
        }
    }
}

void DBN::update_gamma(SearchSession srs) {
    std::vector<SearchResult> s_results = srs.get_sr();
    int qid, did, click, rank;


    for (auto sr: s_results)
    {
        click = sr.get_click();
        rank = sr.get_doc_rank();
        qid = sr.get_query();
        did = sr.get_doc_id();

        DBNFactor factor_func(this->click_probs, this->exam_probs, click,
                              this->last_click_rank, rank,
                              this->attr.get_par(qid, did).value(),
                              this->sat.get_par(qid, did).value(),
                              this->cont.get_par(0, -1).value());
        for (int fct_itr{0}; fct_itr < 8; fct_itr++){
            this->factor_values[fct_itr] = factor_func.compute(this->factor_inputs[fct_itr][0],
                                                               this->factor_inputs[fct_itr][1],this->factor_inputs[fct_itr][2]);
        }

        this->factor_sum = std::accumulate(this->factor_values.begin(), this->factor_values.end(), 0.0);

        double numerator_update = this->factor_values[5] / this->factor_sum;
        double denominator_update = (this->factor_values[4] + this->factor_values[5]) / this->factor_sum;

        this->new_cont.get_par(0, -1).add_to_values(numerator_update, denominator_update);
    }
}

