//
// Created by Pooya Khandel on 06/01/2021.
//

#include <vector>
#include "param.h"


Param::Param() = default;
double Param::value() const{
    return std::min((numerator / denominator), 1 - this->prob_min);
}

double Param::numerator_val() const{
    return this->numerator;
}

double Param::denominator_val() const{
    return this->denominator;
}

void Param::set_values(double numerator_val, double denominator_val){
    this->numerator = numerator_val;
    this->denominator = denominator_val;
}

void Param::add_to_values(double numerator_val, double denominator_val) {
    this->numerator += numerator_val;
    this->denominator += denominator_val;
}


QueryDocumentParamContainer::QueryDocumentParamContainer() = default;

QueryDocumentParamContainer::QueryDocumentParamContainer(const QueryDocumentParamContainer &pc){
    this->qdp = pc.qdp;
}

void QueryDocumentParamContainer::add_qd_par(const int& query_id, const int& doc_id){
    qdp[query_id][doc_id] = Param();
}

Param& QueryDocumentParamContainer::get_par(int qid, int did){
    return qdp[qid][did];
}

void QueryDocumentParamContainer::initialize_for_threads(const Dataset& dataset, const std::vector<int>& train_tasks) {
    for(auto addr: train_tasks){
        for (int rank{0}; rank < 10; rank++){
            SearchResult search_res = dataset.get_search_result(addr + rank);
            this->add_qd_par(search_res.get_query(), search_res.get_doc_id());
        }
    }
}


RankedBasedContainer::RankedBasedContainer() = default;

RankedBasedContainer::RankedBasedContainer(const RankedBasedContainer &pc){
    this->exam = pc.exam;
}

Param& RankedBasedContainer::get_par(int rank_idx_x, int rank_idx_y){
    return exam[rank_idx_x];
}

Param& SingleParamContainer::get_par(int rank_idx_x, int rank_idx_y) {
    return this->single_global_par;
}

SingleParamContainer::SingleParamContainer(const SingleParamContainer &pc){
    this->single_global_par = pc.single_global_par;
}

SingleParamContainer::SingleParamContainer() = default;


RankPrevClickContainer::RankPrevClickContainer() {
    for (int r_x{0}; r_x < 10; r_x++){
        for (int r_y{0}; r_y < 10; r_y++){
            this->exam[r_x][r_y] = Param();
        }
    }
}

RankPrevClickContainer::RankPrevClickContainer(const RankPrevClickContainer &pc) {
    this->exam = pc.exam;
}

Param &RankPrevClickContainer::get_par(int rank_idx_x, int rank_idx_y) {
    return this->exam[rank_idx_x][rank_idx_y];
}
