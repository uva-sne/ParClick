//
// Created by Pooya Khandel on 06/01/2021.
//

#ifndef PBMCPP_SEARCH_H
#define PBMCPP_SEARCH_H

#include <vector>
#include <map>
#include <string>
#include <array>


class SearchResult{
public:
    SearchResult();
    SearchResult(const int& query, const int& doc_id, const int& doc_rank, const int& click);

    void update_click(const int& click_val);
    int get_query() const;
    int get_doc_id() const;
    int get_doc_rank() const;
    int get_click() const;

private:
    int query, doc_id, doc_rank, click;
};


class SearchSession{
public:
    explicit SearchSession(std::vector<SearchResult> sr);
    std::vector<SearchResult>& get_sr();
    int get_last_click_rank();
    int get_num_clicks();
    std::array<int, 10> get_clicks();
    std::array<int, 10> prev_clicked_rank();
private:
    std::vector<SearchResult> sr;
};


class SERP{
public:
    SERP();
    explicit SERP(const std::vector<std::string>& line);
    void update_click_res(const std::vector<std::string>& line);
    SearchResult operator[] (const int& rank) const;
    int tid() const;
    int get_query() const;
private:
    int task_id{-1}, query{}, rank_doc{};
    std::map<int, int> result;
    SearchResult session[10]{};
};


#endif //PBMCPP_SEARCH_H
