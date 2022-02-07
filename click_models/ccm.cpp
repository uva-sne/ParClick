//
// Created by Pooya Khandel on 09/09/2021.
//

#include "ccm.h"

CCM::CCM(const CCM &ccm) {
}

void CCM::say_hello() {
}

void CCM::test_change(int num) {

}

void CCM::init_query_dependent_parameters(const Dataset &dataset, const std::vector<int> &train_tasks) {
    this->attr.initialize_for_threads(dataset, train_tasks);
}

CCM *CCM::clone() {
    return new CCM(*this);
}

Synchronizer *CCM::get_synchronizer(int n_threads) {
    Synchronizer* sync = new CCMSynchronizer(n_threads, 3, 1, 3);
    return sync;
}

void CCM::get_shared_containers(std::vector<ParamContainer*>& temp_container_new,
                                std::vector<ParamContainer*>& temp_container_origin) {
    temp_container_new.push_back(&this->new_cont_no_click);
    temp_container_origin.push_back(&this->cont_no_click);

    temp_container_new.push_back(&this->new_cont_click_non_rel);
    temp_container_origin.push_back(&this->cont_click_non_rel);

    temp_container_new.push_back(&this->new_cont_click_rel);
    temp_container_origin.push_back(&this->cont_click_rel);
}

void CCM::init_temporary_containers() {
    this->base_attr = this->attr;
    this->base_cont_no_click = this->cont_no_click;
    this->base_cont_click_non_rel = this->cont_click_non_rel;
    this->base_cont_click_rel = this->cont_click_rel;
}

void CCM::init_iteration_containers() {
    this->new_attr = this->base_attr;
    this->new_cont_no_click = this->base_cont_no_click;
    this->new_cont_click_non_rel = this->base_cont_click_non_rel;
    this->new_cont_click_rel = this->base_cont_click_rel;
}

void CCM::update_ccm_attr(SearchSession srs)
{
    std::vector<SearchResult> s_results = srs.get_sr();
    double numerator_update, denominator_update;

    int qid, did, click, rank;
    double attr_val, exam_val, car_val;

    for (auto sr: s_results)
    {
        numerator_update = 0;
        denominator_update = 1;

        qid = sr.get_query();
        did = sr.get_doc_id();
        click = sr.get_click();
        rank = sr.get_doc_rank();

        attr_val = this->attr.get_par(qid, did).value();
        exam_val = this->exam[rank];

        if (click == 1) {
            numerator_update += 1;
            denominator_update += 1;
        }
        else if (rank >= this->last_click_rank) {
            car_val = this->car[rank];
            numerator_update += ((1 - exam_val) * attr_val) / ( 1 - exam_val * car_val);
        }

        if (click == 1 and rank == last_click_rank){
            car_val = this->car[rank+1];
            numerator_update += attr_val /
                                (1 -
                                 (this->cont_click_non_rel.get_par(0, -1).value() * ( 1 - attr_val)
                                  + this->cont_click_rel.get_par(0, -1).value() * attr_val)
                                 * car_val);
        }
        this->new_attr.get_par(qid, did).add_to_values(numerator_update, denominator_update);
    }
}

void CCM::process_session(SearchSession search_ses) {
    this->last_click_rank = search_ses.get_last_click_rank();

    this->compute_exam_car(search_ses);

    this->update_ccm_attr(search_ses);

    this->get_tail_clicks(search_ses);

    this->update_taus(search_ses);
}


void CCM::update_thread_specific_parameters() {
    this->attr = this->new_attr;
}

void CCM::get_log_conditional_click_probs(SearchSession search_ses, std::vector<double>& log_click_probs) {
    std::vector<SearchResult> results = search_ses.get_sr();
    SearchResult temp_sr;
    double attr_val, tau_1, tau_2, tau_3;
    double exam_val, click_prob;

    exam_val = 1.0;

    for (int res_itr=0; res_itr < 10; res_itr++){

        temp_sr = results[res_itr];
        attr_val = this->attr.get_par(temp_sr.get_query(), temp_sr.get_doc_id()).value();
        tau_1 = this->cont_no_click.get_par(0, -1).value();
        tau_2 = this->cont_click_non_rel.get_par(0, -1).value();
        tau_3 = this->cont_click_rel.get_par(0, -1).value();

        if (temp_sr.get_click() == 1){
            click_prob = attr_val * exam_val;
            exam_val = tau_2 * (1 - attr_val) + tau_3 * attr_val;
        } else{
            click_prob = 1 - attr_val * exam_val;
            exam_val *= tau_1 * (1 - attr_val) / click_prob;
        }

        log_click_probs.push_back(std::log(click_prob));
    }

}

void CCM::compute_exam_car(SearchSession srs) {
    this->exam[0] = 1;

    int exm_itr = 0;

    int qid, did;
    double attr_val, new_attr_val, tau_1, new_tau_1, tau_2, new_tau_2,
            tau_3, new_tau_3, ex_value, new_ex_value;
    double temp, new_temp, car_val, new_car_val;

    std::vector<SearchResult> srs_vec = srs.get_sr();

    std::vector<std::array<double, 2>> car_helper;
    car_helper.reserve(10);
    std::vector<std::array<double, 10>> new_car_helper;
    new_car_helper.reserve(10);

    for (auto sr: srs_vec)
    {
        qid = sr.get_query();
        did = sr.get_doc_id();

        attr_val = this->attr.get_par(qid, did).value();
        new_attr_val = this->new_attr.get_par(qid, did).value();

        tau_1 = this->cont_no_click.get_par(0, -1).value();
        new_tau_1 = this->new_cont_no_click.get_par(0, -1).value();

        tau_2 = this->cont_click_non_rel.get_par(0, -1).value();
        new_tau_2 = this->new_cont_click_non_rel.get_par(0, -1).value();

        tau_3 = this->cont_click_rel.get_par(0, -1).value();
        new_tau_3 = this->new_cont_click_rel.get_par(0, -1).value();

        ex_value = this->exam[exm_itr];

        temp = (1 - attr_val) * tau_1;
        new_temp = (1 - new_attr_val) * new_tau_1;

        ex_value *= temp + attr_val * ((1 - attr_val) * tau_2 + attr_val * tau_3);
        new_ex_value *= new_temp + new_attr_val * ((1 - new_attr_val) * new_tau_2 + new_attr_val * new_tau_3);

        car_helper[exm_itr][0] = attr_val;
        car_helper[exm_itr][1] = temp;

        new_car_helper[exm_itr][0] = new_attr_val;
        new_car_helper[exm_itr][1] = new_temp;


        exm_itr += 1;

        this->exam[exm_itr] = ex_value;
    }

    this->car = {0};
    for (int car_itr=9; car_itr > -1; car_itr--)
    {
        car_val = this->car[car_itr+1];

        this->car[car_itr] = car_helper[car_itr][0] + car_helper[car_itr][1] * car_val;
    }

}

void CCM::get_tail_clicks(SearchSession srs) {
    std::vector<SearchResult> results = srs.get_sr();
    this->exam_probs[0] = 1.0;
    SearchResult temp_sr;
    double tau_1, tau_2, tau_3;

    double exam_val, click_prob;
    for (int start_rank = 0; start_rank < 10; start_rank++){
        exam_val = 1.0;
        int ses_itr{0};
        for (int res_itr=start_rank; res_itr < 10; res_itr++){
            temp_sr = results[ses_itr];
            double attr_val = this->attr.get_par(temp_sr.get_query(), temp_sr.get_doc_id()).value();
            tau_1 = this->cont_no_click.get_par(0, -1).value();
            tau_2 = this->cont_click_non_rel.get_par(0, -1).value();
            tau_3 = this->cont_click_rel.get_par(0, -1).value();

            if (results[res_itr].get_click() == 1){
                click_prob = attr_val * exam_val;
                exam_val = tau_2 * (1 - attr_val) + tau_3 * attr_val;
            } else{
                click_prob = 1 - attr_val * exam_val;
                exam_val *= tau_1 * (1 - attr_val) / click_prob;
            }
            this->click_probs[start_rank][ses_itr] = click_prob;
            if (start_rank == 0){
                this->exam_probs[ses_itr+1] = exam_val;
            }
            ses_itr++;
        }

    }
}

void CCM::update_taus(SearchSession srs) {
    std::vector<SearchResult> s_results = srs.get_sr();

    int qid, did, click, rank;

    for (auto sr: s_results)
    {
        click = sr.get_click();
        rank = sr.get_doc_rank();
        qid = sr.get_query();
        did = sr.get_doc_id();

        CCMFactor factor_func(this->click_probs, this->exam_probs, click,
                              this->last_click_rank, rank,
                              this->attr.get_par(qid, did).value(),
                              this->cont_no_click.get_par(0, -1).value(),
                              this->cont_click_non_rel.get_par(0, -1).value(),
                              this->cont_click_rel.get_par(0, -1).value());


        for (int fct_itr{0}; fct_itr < 8; fct_itr++){
            this->factor_values[fct_itr] = factor_func.compute(this->factor_inputs[fct_itr][0],
                                                               this->factor_inputs[fct_itr][1],this->factor_inputs[fct_itr][2]);
        }

        this->factor_sum = std::accumulate(this->factor_values.begin(), this->factor_values.end(), 0.0);

        if (click == 0){
            this->update_tau_1();
        }else{
            this->update_tau_2();
            this->update_tau_3();
        }
    }
}

void CCM::update_tau_1() {
    double numerator_update{(this->factor_values[5] + this->factor_values[7]) / this->factor_sum};
    double denominator_update{numerator_update + ((this->factor_values[4] + this->factor_values[6]) / this->factor_sum)};
    this->new_cont_no_click.get_par(0, -1).add_to_values(numerator_update, denominator_update);
}

void CCM::update_tau_2() {
    double numerator_update{this->factor_values[5] / this->factor_sum};
    double denominator_update{numerator_update + ((this->factor_values[4]) / this->factor_sum)};
    this->new_cont_click_non_rel.get_par(0, -1).add_to_values(numerator_update, denominator_update);
}

void CCM::update_tau_3() {
    double numerator_update{this->factor_values[7] / this->factor_sum};
    double denominator_update{numerator_update + ((this->factor_values[6]) / this->factor_sum)};
    this->new_cont_click_rel.get_par(0, -1).add_to_values(numerator_update, denominator_update);
}

void CCM::get_full_click_probs(SearchSession search_ses, std::vector<double> &full_click_probs) {
    std::vector<SearchResult> sr_vec = search_ses.get_sr();
    double atr_val, tau_1, tau_2, tau_3, atr_mul_ex;
    double  ex_value{1};


    for (int rank = 0; rank < 10; rank++) {
        SearchResult session = sr_vec[rank];
        atr_val = this->attr.get_par(session.get_query(), session.get_doc_id()).value();
        tau_1 = this->cont_no_click.get_par(0, -1).value();
        tau_2 = this->cont_click_non_rel.get_par(0, -1).value();
        tau_3 = this->cont_click_rel.get_par(0, -1).value();

        atr_mul_ex = atr_val * ex_value;

        int click{session.get_click()};
        if (click == 1) {
            full_click_probs.push_back(atr_mul_ex);
        }
        else {
            full_click_probs.push_back(1 - atr_mul_ex);
        }

        ex_value *= (1 - atr_val) * tau_1 + atr_val * ((1 - atr_val) * tau_2 + atr_val * tau_3);

    }
}
