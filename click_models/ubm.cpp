//
// Created by Pooya Khandel on 01/11/2021.
//

#include "ubm.h"

UBM::UBM() = default;

UBM::UBM(const UBM &ubm) {

}

void UBM::say_hello() {
    std::cout << "UBM says hello!" << std::endl;
}

void UBM::test_change(int num) {

}

UBM *UBM::clone() {
    return new UBM(*this);
}

void UBM::init_query_dependent_parameters(const Dataset &dataset, const std::vector<int> &train_tasks) {
    this->attr.initialize_for_threads(dataset, train_tasks);
}

Synchronizer *UBM::get_synchronizer(int n_threads) {
    Synchronizer* sync = new UBMSynchronizer(n_threads, 10, 1, 1);
    return sync;
}

void UBM::get_shared_containers(std::vector<ParamContainer*>& temp_container_new,
                                std::vector<ParamContainer*>& temp_container_origin) {
    temp_container_new.push_back(&this->new_exams);
    temp_container_origin.push_back(&this->exams);
}

void UBM::init_temporary_containers() {
    this->base_attr = this->attr;
    this->base_exams = this->exams;
}

void UBM::init_iteration_containers() {
    this->new_attr = this->base_attr;
    this->new_exams = this->base_exams;
}

void UBM::update_thread_specific_parameters() {
    this->attr = this->new_attr;
}

void UBM::process_session(SearchSession search_ses) {
    std::vector<SearchResult> sr_vec = search_ses.get_sr();
    std::array<int, 10> prev_click_rank = search_ses.prev_clicked_rank();
    for (int rank=0; rank < 10; rank++){
        SearchResult sr = sr_vec[rank];
        int qid{sr.get_query()};
        int did{sr.get_doc_id()};
        int dr{sr.get_doc_rank()};

        double cur_atr_value{this->attr.get_par(qid, did).value()};
        double cur_ex_value{this->exams.get_par(dr, prev_click_rank[dr]).value()};


        double new_numerator_atr{1};
        double new_numerator_ex{1};

        if (sr.get_click() == 0){
            double atr_ex = cur_atr_value * cur_ex_value;
            new_numerator_atr = (cur_atr_value - atr_ex) / (1 - atr_ex);
            new_numerator_ex = (cur_ex_value - atr_ex) / (1 - atr_ex);
        }

        this->new_attr.get_par(qid, did).add_to_values(new_numerator_atr, 1);
        this->new_exams.get_par(dr, prev_click_rank[dr]).add_to_values(new_numerator_ex, 1);
    }
}

void UBM::get_log_conditional_click_probs(SearchSession search_ses, std::vector<double>& log_click_probs) {
    std::vector<SearchResult> sr_vec = search_ses.get_sr();
    std::array<int, 10> prev_click_rank = search_ses.prev_clicked_rank();
    for (int rank = 0; rank < 10; rank++) {
        SearchResult session = sr_vec[rank];
        double atr{this->attr.get_par(session.get_query(), session.get_doc_id()).value()};
        double ex{this->exams.get_par(session.get_doc_rank(), prev_click_rank[rank]).value()};
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

void UBM::get_full_click_probs(SearchSession search_ses, std::vector<double> &full_click_probs) {
    std::vector<SearchResult> sr_vec = search_ses.get_sr();
    std::vector<double> temp_full_click_probs;
    std::array<int, 10> prev_click_rank = search_ses.prev_clicked_rank();
    for (int rank = 0; rank < 10; rank++) {
        double click_prob{0};
        SearchResult session = sr_vec[rank];


        for (int rank_prev_click{-1}; rank_prev_click < rank; rank_prev_click++){
            double no_click_between = 1.0;
            int corrected_rank_prev_click{rank_prev_click};
            if (rank_prev_click == -1){
                corrected_rank_prev_click = 9;
            }
            for (int rank_between{rank_prev_click+1}; rank_between < rank; rank_between++){
                double attr_val{this->attr.get_par(sr_vec[rank_between].get_query(), sr_vec[rank_between].get_doc_id()).value()};
                double exam_val{this->exams.get_par(rank_between, corrected_rank_prev_click).value()};
                no_click_between *= 1 - attr_val * exam_val;
            }
            double attr_val{this->attr.get_par(sr_vec[rank].get_query(), sr_vec[rank].get_doc_id()).value()};
            double exam_val{this->exams.get_par(rank, corrected_rank_prev_click).value()};
            double temp{no_click_between * (attr_val * exam_val)};
            if (rank_prev_click >= 0){
                click_prob += temp_full_click_probs[rank_prev_click] * temp;
            } else{
                click_prob += temp;
            }
        }
        temp_full_click_probs.push_back(click_prob);
        if (session.get_click() == 1) {
            full_click_probs.push_back(click_prob);
        }
        else {
            full_click_probs.push_back(1 - click_prob);
        }
    }

}

