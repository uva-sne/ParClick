//
// Created by Pooya Khandel on 09/09/2021.
//

#ifndef CASCADE_PARALLEL_FACTOR_H
#define CASCADE_PARALLEL_FACTOR_H
#include "base.h"

class CCMFactor{
private:
    int click, last_click_rank, rank;
    double attr, tau_1, tau_2, tau_3;
    std::map<int, std::map<int, double>> click_probs;
    std::array<double, 11> exam_probs{};
public:
    CCMFactor(std::map<int, std::map<int, double>>& click_probs, std::array<double, 11>& exam_probs, int click, int last_click_rank, int rank, double attr, double tau_1, double tau_2, double tau_3);
    double compute(int x, int y, int z);
};


class DBNFactor{
private:
    int click, last_click_rank, rank;
    double attr, sat, gamma;
    std::map<int, std::map<int, double>> click_probs;
    std::array<double, 11> exam_probs{};
public:
    DBNFactor(std::map<int, std::map<int, double>>& click_probs, std::array<double, 11>& exam_probs, int click, int last_click_rank, int rank, double attr, double sat, double gamma);
    double compute(int x, int y, int z);
};


#endif //CASCADE_PARALLEL_FACTOR_H
