//
// Created by Pooya Khandel on 09/09/2021.
//

#include "factor.h"


CCMFactor::CCMFactor(std::map<int, std::map<int, double>>& click_probs, std::array<double, 11>& exam_probs, int click, int last_click_rank, int rank, double attr, double tau_1, double tau_2, double tau_3) {
    this->click_probs = click_probs;
    this->exam_probs = exam_probs;
    this->click = click;
    this->last_click_rank = last_click_rank;
    this->rank = rank;
    this->attr = attr;
    this->tau_1 = tau_1;
    this->tau_2 = tau_2;
    this->tau_3 = tau_3;
}

double CCMFactor::compute(int x, int y, int z) {
    double log_prob = 0.0;

    if (this->click == 0)
    {
        if (y == 1)
            return 0.0;

        log_prob += std::log(1 - this->attr);
        if (x == 1)
        {
            if ( z == 1)
                log_prob += std::log(this->tau_1);
            else
                log_prob += std::log(1 - this->tau_1);
        }
        else if (z == 1)
            return 0.0;
    }
    else
    {
        if (x == 0)
            return 0.0;

        log_prob += std::log(this->attr);
        if (y == 0)
        {
            log_prob += std::log(1 - this->attr);
            if (z == 1)
                log_prob += std::log(this->tau_2);
            else
                log_prob += std::log(1 - this->tau_2);

        }
        else
        {
            log_prob += std::log(this->attr);
            if (z == 1)
                log_prob += std::log(this->tau_3);
            else
                log_prob += std::log(1 - this->tau_3);

        }
    }

    if (z == 0){
        if (this->last_click_rank >= (this->rank + 1))
            return 0.0;
    }
    else if (this->rank + 1 < 10){
        std::map<int, double> tail_clicks = this->click_probs[this->rank+1];
//        for (auto& element: tail_clicks){
//            log_prob += std::log(element.second);
//        }
        log_prob += std::accumulate(tail_clicks.begin(), tail_clicks.end(), 0.0, [](const double base, decltype(*begin(tail_clicks))& element){return base + std::log(element.second);});
//        log_prob += std::accumulate(this->click_probs[this->rank+1].begin(), this->click_probs[this->rank+1].end(),
//                                    0, [](double base, const auto &tail_clicks)
//                                    {return base + std::log(tail_clicks.second);});

    }

    double exam_val = this->exam_probs[this->rank];
    if (x == 1){
        log_prob += std::log(exam_val);

    } else{
        log_prob += std::log(1 - exam_val);

    }
    return std::exp(log_prob);
}

DBNFactor::DBNFactor(std::map<int, std::map<int, double>> &click_probs, std::array<double, 11> &exam_probs, int click,
                     int last_click_rank, int rank, double attr, double sat, double gamma) {
    this->click_probs = click_probs;
    this->exam_probs = exam_probs;
    this->click = click;
    this->last_click_rank = last_click_rank;
    this->rank = rank;
    this->attr = attr;
    this->gamma = gamma;
    this->sat = sat;
}

double DBNFactor::compute(int x, int y, int z) {
    double log_prob = 0.0;
    if (this->click == 0){
        if (y == 1){
            return 0.0;
        }
        log_prob += std::log(1 - this->attr);
        if (x == 1){
            if (z == 1){
                log_prob += std::log(this->gamma);
            }else{
                log_prob += std::log(1 - this->gamma);
            }
        } else if (z == 1){
            return 0.0;
        }
    } else{
        if (x == 0){
            return 0.0;
        }
        log_prob += std::log(this->attr);
        if (y == 0){
            log_prob += std::log(1 - this->sat);
            if (z == 1){
                log_prob += std::log(this->gamma);
            }else{
                log_prob += std::log(1- this->gamma);
            }
        }else{
            if (z == 1){
                return 0.0;
            }
            log_prob += std::log(this->sat);
        }
    }


    if (z == 0){
        if (this->last_click_rank >= this->rank + 1){
            return 0.0;
        }
    } else if (this->rank + 1 < 10){
        std::map<int, double> tail_clicks = this->click_probs[this->rank+1];
        log_prob += std::accumulate(tail_clicks.begin(), tail_clicks.end(), 0.0, [](const double base, decltype(*begin(tail_clicks))& element){return base + std::log(element.second);});
    }
    double exam_val = this->exam_probs[this->rank];
    if (x == 1){
        log_prob += std::log(exam_val);

    } else{
        log_prob += std::log(1 - exam_val);

    }

    return std::exp(log_prob);
}
