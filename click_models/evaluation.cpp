//
// Created by Pooya Khandel on 30/09/2021.
//

#include "evaluation.h"

LogLikelihood::LogLikelihood(ClickModel *cm) {
    this->cm = cm;

}

double LogLikelihood::evaluate(Dataset &pr_dataset, ThreadPartition &tasks) {
    for (auto idx: tasks.thread_tasks(false)) {
        std::vector<double> log_click_probs;
        log_click_probs.reserve(10);

        SearchSession sr = pr_dataset.get_search_session(idx);
        this->cm->get_log_conditional_click_probs(sr, log_click_probs);

        this->llh_values.push_back(std::accumulate(log_click_probs.begin(), log_click_probs.end(), 0.0) / 10.0);
    }
    return std::accumulate(llh_values.begin(), llh_values.end(),
                           0.0);
}

void Perplexity::evaluate(ClickModel* cm, Dataset &pr_dataset, ThreadPartition &tasks) {
    this->task_size = static_cast<double>(tasks.thread_tasks(false).size());

    for (auto idx: tasks.thread_tasks(false)) {
        std::vector<double> full_click_probs;
        full_click_probs.reserve(10);

        const SearchSession sr = pr_dataset.get_search_session(idx);
        cm->get_full_click_probs(sr, full_click_probs);

        for (int i{0}; i < 10; i++){
            this->task_rank_perplexities[i] += std::log2(full_click_probs[i]);
        }
    }
}

Perplexity::Perplexity(const Perplexity &ppl_obj) {
    this->task_size = ppl_obj.task_size;
    this->task_rank_perplexities = ppl_obj.task_rank_perplexities;
}
