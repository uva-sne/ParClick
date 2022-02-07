//
// Created by Pooya Khandel on 06/01/2021.
//

#include "partition.h"


ThreadPartition::ThreadPartition() = default;
ThreadPartition::ThreadPartition(double test_share){
    this->test_share = test_share;
}

void ThreadPartition::add_train(const std::vector<int>& addr, int query_id){
    this->train_queries.insert(query_id);
    for (auto an_addr: addr){
        this->train_idx.push_back(an_addr);
    }
}

std::vector<int> ThreadPartition::thread_tasks(bool return_train){
    if (return_train)
    {
        return this->train_idx;
    }
    else{
        return this->test_idx;
    }
}

void ThreadPartition::filter_test_unseen() {
    std::unordered_set<size_t> filtered_test;
    for (auto query: this->test_queries){
        if (train_queries.find(query) == train_queries.end()){
            // test query not seen
            std::vector<int> invlaid_addrs = this->test_query_vs_addr[query];
            for (auto addr: invlaid_addrs){
                test_idx.erase(std::remove(test_idx.begin(), test_idx.end(), addr),
                               test_idx.end());
            }

        } else{
            filtered_test.insert(query);
        }
    }
    this->test_queries = filtered_test;

}

size_t ThreadPartition::size_queries(bool return_train) {
    if (return_train){
        return train_queries.size();
    } else{
        return test_queries.size();
    }
}

size_t ThreadPartition::size_sessions(bool return_train) {
    if (return_train){
        return train_idx.size();
    } else{
        return test_idx.size();
    }
}

bool ThreadPartition::query_in_train(int query_id) {
    if (this->train_queries.find(query_id) == this->train_queries.end()){
        return false;
    }
    else{
        return true;
    }
}

void ThreadPartition::add_test(const std::vector<int> &addr, int query_id) {
    this->test_queries.insert(query_id);
    for (auto an_addr: addr){
        this->test_idx.push_back(an_addr);
    }
}

void ThreadPartition::sort_train() {
    std::sort(this->train_idx.begin(), this->train_idx.end());
}

int ThreadPartition::size() {
    return this->train_idx.size();
}

int ThreadPartition::size_test() {
    return this->test_idx.size();
}

void ThreadPartition::remove_from_test_index(Dataset& dataset, int N1, int N2) {
    std::cout << this->test_idx.size() << std::endl;
    this->test_idx.erase(std::remove_if(this->test_idx.begin(), this->test_idx.end(), [&dataset, &N1, &N2](int adr){
        int query = dataset.get_search_result(adr).get_query();
        int freq = dataset.get_query_freq(query);
        if (freq < N1){
            return true;
        }
        else{
            return false;
        }
    }), this->test_idx.end());
    std::cout << this->test_idx.size() << std::endl;
}

Partition::Partition(int n_thread, double test_share){
    this->n_thread = n_thread;
    for (int i = 0; i < n_thread; i++) {
        this->partition_map[i] = ThreadPartition(test_share);
        this->thread_utilization[i] = 0;
    }
}

void Partition::add_train(int row_addr, const std::vector<int>& qid_addr, int query_id){
    int tid = row_addr % this->n_thread;
    this->partition_map[tid].add_train(qid_addr, query_id);
}

void MUPartition::add_train(int row_addr, const std::vector<int>& qid_addr, int query_id){
    int tid = this->get_less_utilized_partition();
    this->partition_map[tid].add_train(qid_addr, query_id);
    this->thread_utilization[tid] += qid_addr.size();
}

int MUPartition::get_less_utilized_partition(){
    std::pair<int, int> less_utilized_partition = *std::min_element(this->thread_utilization.begin(), this->thread_utilization.end(), &MUPartition::compare_size);
    return less_utilized_partition.first;
}

bool MUPartition::compare_size(std::pair<int, int> t1, std::pair<int, int> t2){
    return t1.second < t2.second;
}

MUPartition::MUPartition(int n_thread, double test_share) : Partition(n_thread, test_share) {
    this->n_thread = n_thread;
    for (int i = 0; i < n_thread; i++) {
        this->partition_map[i] = ThreadPartition(test_share);
        this->thread_utilization[i] = 0;
    }
}

ThreadPartition& Partition::get_pr_part(int tid){
    return partition_map[tid];
}

void Partition::add_test(int row_addr, const std::vector<int> &qid_addr, int query_id, bool filter) {
    if (!filter){
        int tid = row_addr % this->n_thread;
        this->partition_map[tid].add_test(qid_addr, query_id);
    }
    else {
        bool found{false};
        int partition_itr{0};
        while (!found && partition_itr < partition_map.size()){
            if (this->partition_map[partition_itr].query_in_train(query_id)){
                found = true;
                this->partition_map[partition_itr].add_test(qid_addr, query_id);
            }
            partition_itr++;
        }
    }
}
