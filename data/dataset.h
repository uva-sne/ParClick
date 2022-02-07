//
// Created by Pooya Khandel on 06/01/2021.
//

#ifndef PBMCPP_DATASET_H
#define PBMCPP_DATASET_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <iterator>
#include <cstdlib>
#include "search.h"
#include "partition.h"

class Partition;

class Dataset {
public:
    Dataset();
    int size();
    // Add each result of search session as a separate row
    void add_session(const SERP& ses, int base_adr);
    SearchSession get_search_session(int addr);
    // Returns search result of addr
    SearchResult get_search_result(int addr) const;
    // Scatter data between threads, split based on query
    void make_splits(int n_threads, double test_share, bool filter, Partition** part);
    int get_query_freq(int adr);
private:
    // This keeps track of the address if queries in `qu_doc`
    std::map<int, std::vector<int>> ses_adr_train;
    std::map<int, std::vector<int>> ses_adr_test;
    std::vector<SearchResult> qu_doc;
};

void parse_dataset(Dataset& dataset, const std::string& raw_dataset_path,
                   int max_sessions=-1);

#endif //PBMCPP_DATASET_H
