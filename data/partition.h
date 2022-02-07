//
// Created by Pooya Khandel on 06/01/2021.
//

#ifndef PBMCPP_PARTITION_H
#define PBMCPP_PARTITION_H

#include <vector>
#include <map>
#include <random>
#include <unordered_set>
#include <algorithm>
#include "dataset.h"

class Dataset;

class ThreadPartition{
private:
    std::vector<int> train_idx;
    std::unordered_set<size_t> train_queries;
    std::unordered_set<size_t> test_queries;
    std::vector<int> test_idx;
    std::map<size_t, std::vector<int>> test_query_vs_addr;
    double test_share{0.2};
    std::default_random_engine generator;
public:
    ThreadPartition();
    explicit ThreadPartition(double test_share);
    void filter_test_unseen();
    void add_train(const std::vector<int>& addr, int query_id);
    void add_test(const std::vector<int>& addr, int query_id);
    void remove_from_test_index(Dataset& dataset, int N1, int N2);
    std::vector<int> thread_tasks(bool return_train=true);
    size_t size_queries(bool return_train);
    size_t size_sessions(bool return_train);
    bool query_in_train(int query_id);
    int size();
    int size_test();
    void sort_train();
};


class Partition{
public:
    int n_thread;
    std::map<int, int> thread_utilization;
    std::map<int, ThreadPartition> partition_map;
    explicit Partition(int n_thread, double test_share);
    virtual void add_train(int row_addr, const std::vector<int>& qid_addr, int query_id);
    void add_test(int row_addr, const std::vector<int>& qid_addr, int query_id, bool filter);
    ThreadPartition& get_pr_part(int tid);
};

class MUPartition: public Partition{
public:
    explicit MUPartition(int n_thread, double test_share);
    void add_train(int row_addr, const std::vector<int>& qid_addr, int query_id) override;
    int get_less_utilized_partition();
    static bool compare_size(std::pair<int, int> t1, std::pair<int, int> t2);
};


#endif //PBMCPP_PARTITION_H
