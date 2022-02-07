//
// Created by Pooya Khandel on 06/01/2021.
//

#include "dataset.h"

Dataset::Dataset() = default;

int Dataset::size(){
    return static_cast<int>(qu_doc.size()) / 10;
}

void Dataset::add_session(const SERP& ses, int base_adr){
    for (int i=0; i < 10; i++){
        qu_doc.push_back(ses[i]);
    }
}

SearchResult Dataset::get_search_result(int addr) const{
    return qu_doc[addr];
}

void Dataset::make_splits(int n_threads, double test_share, bool filter, Partition** part){
    std::map<int, std::vector<int>>::iterator adr_itr;
    int train_top_addr{static_cast<int>(this->size() * (1 - test_share) * 10)};

    int addr{0};
    while (addr < train_top_addr){
        SearchResult sr = this->get_search_result(addr);
        ses_adr_train[sr.get_query()].push_back(addr);
        addr += 10;
    }

    int row_index{0};

    std::vector<int> dropped_sessions_adr;
    for (adr_itr = ses_adr_train.begin(); adr_itr != ses_adr_train.end(); adr_itr++){
        (*part)->add_train(row_index++, adr_itr->second, adr_itr->first);
    }

    while (addr < qu_doc.size()){
        SearchResult sr = this->get_search_result(addr);
        ses_adr_test[sr.get_query()].push_back(addr);
        addr +=10;
    }


    row_index = 0;
    for (adr_itr = ses_adr_test.begin(); adr_itr != ses_adr_test.end(); adr_itr++){
        (*part)->add_test(row_index++, adr_itr->second, adr_itr->first, filter);
    }

}

SearchSession Dataset::get_search_session(int addr) {
    std::vector<SearchResult> sr;
    sr.reserve(10);
    for (int  rank=0; rank < 10; rank++)
    {
        sr.push_back(this->get_search_result(addr + rank));
    }
    return SearchSession(sr);
}

int Dataset::get_query_freq(int query) {
    return this->ses_adr_test[query].size();
}

void parse_dataset(Dataset& dataset, const std::string& raw_dataset_path, int max_sessions){
    std::ifstream raw_file(raw_dataset_path);
    std::string line;
    std::string element;
    int itr_session{0};
    if (raw_file.is_open()){
        std::cout << "Raw dataset file is opened ...!" << std::endl;

        SERP prev_ses, cur_ses;


        while (std::getline(raw_file, line) && (itr_session < max_sessions || max_sessions == -1)) {

            std::stringstream ssi(line);
            std::vector<std::string> line_vec;
            while (ssi.good()){
                ssi >> element;
                line_vec.push_back(element);
            }

            if(line_vec.size() == 15){
                if(prev_ses.tid() != -1){
                    dataset.add_session(prev_ses, itr_session);
                    itr_session++;
                }
                cur_ses = SERP(line_vec);
            }
            else if (line_vec.size() == 4){
                cur_ses.update_click_res(line_vec);
            }
            else{
                std::cout << "InValid data: " << line;
            }
            prev_ses = cur_ses;
        }

        std::cout << "Number of session available: " << itr_session << std::endl;
    }
    else{
        std::cout << "Unable to open the raw dataset ... !" << std::endl;
    }

}