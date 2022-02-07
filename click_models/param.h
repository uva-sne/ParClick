//
// Created by Pooya Khandel on 06/01/2021.
//

#ifndef PBMCPP_PARAM_H
#define PBMCPP_PARAM_H
#include <mutex>
#include <map>

#include "data/dataset.h"

class Param{
private:
    double numerator{1};
    double denominator{2};
    double prob_min{0.000001};
public:
    Param();
    double value() const;
    double numerator_val() const;
    double denominator_val() const;
    void set_values(double numerator_val, double denominator_val);
    void add_to_values(double numerator_val, double denominator_val);
};


class ParamContainer{
public:
    virtual Param& get_par(int idx_x, int idx_y) = 0;
    virtual ~ParamContainer() = default;
};


class QueryDocumentParamContainer: ParamContainer{
private:
    std::map<int, std::map<int, Param>> qdp;
public:
    QueryDocumentParamContainer();
    QueryDocumentParamContainer(const QueryDocumentParamContainer &pc);
    void initialize_for_threads(const Dataset& dataset, const std::vector<int>& train_tasks);
    void add_qd_par(const int& query_id, const int& doc_id);
    Param& get_par(int qid, int did) override;
};

class RankedBasedContainer: public ParamContainer{
private:
    std::map<int, Param> exam{{0, Param()},
                              {1, Param()},
                              {2, Param()},
                              {3, Param()},
                              {4, Param()},
                              {5, Param()},
                              {6, Param()},
                              {7, Param()},
                              {8, Param()},
                              {9, Param()}};
public:
    RankedBasedContainer();
    RankedBasedContainer(const RankedBasedContainer &pc);
    Param& get_par(int rank_idx_x, int rank_idx_y) override;
    ~RankedBasedContainer() override = default;
};

class RankPrevClickContainer: public ParamContainer{
private:
    std::map<int, std::map<int, Param>> exam;
public:
    RankPrevClickContainer();
    RankPrevClickContainer(const RankPrevClickContainer &pc);
    Param& get_par(int rank_idx_x, int rank_idx_y) override;
    ~RankPrevClickContainer() override = default;
};

class SingleParamContainer: public ParamContainer{
public:
    SingleParamContainer(const SingleParamContainer &pc);
    Param& get_par(int rank_idx_x, int rank_idx_y) override;
    SingleParamContainer();
    ~SingleParamContainer() override = default;
private:
    Param single_global_par;
};


#endif //PBMCPP_PARAM_H
