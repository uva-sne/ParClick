//
// Created by Pooya Khandel on 09/09/2021.
//

#include "pbm.h"


PBM *PBM::clone() {
    return new PBM(*this);
}

void PBM::init_query_dependent_parameters(const Dataset &dataset, const std::vector<int> &train_tasks) {
    this->attr.initialize_for_threads(dataset, train_tasks);
}

void PBM::say_hello() {
}

void PBM::test_change(int num) {
}

Synchronizer *PBM::get_synchronizer(int n_threads) {
    Synchronizer* sync = new PBMSynchronizer(n_threads, 10, 1, 1);
    return sync;
}

void  PBM::get_shared_containers(std::vector<ParamContainer*>& temp_container_new,
                                 std::vector<ParamContainer*>& temp_container_origin) {
    temp_container_new.push_back(&this->new_exams);
    temp_container_origin.push_back(&this->exams);
}

PBM::PBM() = default;

PBM::PBM(PBM const &pbm) {
}

void PBM::init_temporary_containers() {
    this->base_attr = this->attr;
    this->base_exams = this->exams;
}

void PBM::init_iteration_containers() {
    this->new_attr = this->base_attr;
    this->new_exams = this->base_exams;
}

void PBM::process_session(SearchSession search_ses) {
    std::vector<SearchResult> sr_vec = search_ses.get_sr();
    for (int rank=0; rank < 10; rank++){
        SearchResult sr = sr_vec[rank];
        int qid{sr.get_query()};
        int did{sr.get_doc_id()};
        int dr{sr.get_doc_rank()};

        double cur_atr_value{this->attr.get_par(qid, did).value()};
        double cur_ex_value{this->exams.get_par(dr, -1).value()};

        double new_numerator_atr{1};
        double new_numerator_ex{1};

        if (sr.get_click() == 0){
            double atr_ex = cur_atr_value * cur_ex_value;
            new_numerator_atr = (cur_atr_value - atr_ex) / (1 - atr_ex);
            new_numerator_ex = (cur_ex_value - atr_ex) / (1 - atr_ex);
        }

        this->new_attr.get_par(qid, did).add_to_values(new_numerator_atr, 1);
        this->new_exams.get_par(dr, -1).add_to_values(new_numerator_ex, 1);
    }
}

void PBM::update_thread_specific_parameters() {
    this->attr = this->new_attr;
}

void PBM::get_log_conditional_click_probs(SearchSession search_ses, std::vector<double>& log_click_probs) {
    std::vector<SearchResult> sr_vec = search_ses.get_sr();
    for (int rank = 0; rank < 10; rank++) {
        SearchResult session = sr_vec[rank];
        double atr{this->attr.get_par(session.get_query(), session.get_doc_id()).value()};
        double ex{this->exams.get_par(session.get_doc_rank(), -1).value()};
        double atr_mul_ex = atr * ex;
        int click{session.get_click()};
        if (click == 1) {
            log_click_probs.push_back(std::log(atr_mul_ex));
        }
        else {
            log_click_probs.push_back(std::log(1 - atr_mul_ex));
        }
    }
}

void PBM::get_full_click_probs(SearchSession search_ses, std::vector<double> &full_click_probs) {
    std::vector<SearchResult> sr_vec = search_ses.get_sr();
    for (int rank = 0; rank < 10; rank++) {
        SearchResult session = sr_vec[rank];
        double atr{this->attr.get_par(session.get_query(), session.get_doc_id()).value()};
        double ex{this->exams.get_par(session.get_doc_rank(), -1).value()};
        double atr_mul_ex = atr * ex;
        int click{session.get_click()};
        if (click == 1) {
            full_click_probs.push_back(atr_mul_ex);
        }
        else {
            full_click_probs.push_back(1 - atr_mul_ex);
        }
    }
}

