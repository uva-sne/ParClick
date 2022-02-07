//
// Created by Pooya Khandel on 30/09/2021.
//

#ifndef CASCADE_PARALLEL_EVALUATION_H
#define CASCADE_PARALLEL_EVALUATION_H
#include <iostream>
#include <vector>
#include "click_models/base.h"


class LogLikelihood{
private:
    std::vector<double> llh_values;
    ClickModel* cm;
public:
    explicit LogLikelihood(ClickModel* cm);
    double evaluate(Dataset& pr_dataset, ThreadPartition& tasks);
};


class Perplexity{
public:
    std::array<double, 10> task_rank_perplexities{{0}};
    double task_size{0};
    Perplexity() = default;
    Perplexity(Perplexity const &ppl_obj);
    void evaluate(ClickModel* cm, Dataset& pr_dataset, ThreadPartition& tasks);
};
#endif //CASCADE_PARALLEL_EVALUATION_H
