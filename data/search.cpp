//
// Created by Pooya Khandel on 06/01/2021.
//

#include "search.h"

#include <utility>

SearchResult::SearchResult() = default;
SearchResult::SearchResult(const int& query, const int& doc_id, const int& doc_rank, const int& click){
    this->query = query;
    this->doc_id = doc_id;
    this->doc_rank = doc_rank;
    this->click = click;
}

void SearchResult::update_click(const int& click_val){
    this->click = click_val;
}

int SearchResult::get_query() const{
    return query;
}

int SearchResult::get_doc_id() const{
    return doc_id;
}

int SearchResult::get_doc_rank() const{
    return doc_rank;
}

int SearchResult::get_click() const{
    return click;
}

SERP::SERP()= default;
SERP::SERP(const std::vector<std::string>& line) {
    query = std::stoi(line[3]);
    task_id = std::stoi(line[0]);
    for(int i=0; i < 10; i++) {
        rank_doc = std::stoi(line[i + 5]);
        result[rank_doc] = i;
        session[i] = SearchResult(query, rank_doc, i, 0);
    }
}
void SERP::update_click_res(const std::vector<std::string>& line){
    if(task_id == std::stoi(line[0])){
        int doc_id = std::stoi(line[3]);
        auto rank_itr = result.find(doc_id);
        if (rank_itr != result.end()){
            int rank = rank_itr->second;
            session[rank].update_click(1);
        }
    }
}

SearchResult SERP::operator[] (const int& rank) const{
    return session[rank];
}

int SERP::tid() const{
    return task_id;
}

int SERP::get_query() const{
    return query;
}

SearchSession::SearchSession(std::vector<SearchResult> sr) {
    this->sr = std::move(sr);
}

std::vector<SearchResult>& SearchSession::get_sr() {
    return this->sr;
}

int SearchSession::get_last_click_rank() {
    int last_click_rank{10};
    for (auto asr: this->sr)
    {
        if (asr.get_click() == 1)
        {
            last_click_rank = asr.get_doc_rank();
        }
    }
    return last_click_rank;
}

int SearchSession::get_num_clicks() {
    int click_cnt{0};
    for (auto asr: this->sr)
    {
        if (asr.get_click() == 1)
        {
            click_cnt++;
        }
    }
    return click_cnt;
}

std::array<int, 10> SearchSession::prev_clicked_rank() {
    std::vector<int> prev_clicks;
    std::array<int, 10> prev_click_rank{0};
    std::array<int, 10> clicks = this->get_clicks();

    for (int rank_cnt{0}; rank_cnt < 10; rank_cnt++){
        int max_itr_rank = rank_cnt;
        int itr{0};
        std::vector<int> temp_clicks;
        for (auto a_click: clicks){
            if (itr < max_itr_rank){
                if (a_click == 1){
                    temp_clicks.push_back(itr);
                }
                itr++;
            }
        }
        if (!temp_clicks.empty()){
            prev_click_rank[rank_cnt] = temp_clicks.back();
        } else{
            prev_click_rank[rank_cnt] = 9;
        }
    }

    return prev_click_rank;
}

std::array<int, 10> SearchSession::get_clicks() {
    std::array<int, 10> clicks{0};
    int cnt{0};
    for (auto a_sr: this->sr){
        clicks[cnt] = a_sr.get_click();
        cnt++;
    }
    return clicks;
}
