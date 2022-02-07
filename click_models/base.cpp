//
// Created by Pooya Khandel on 15/04/2021.
//

#include "base.h"
#include "pbm.h"
#include "ccm.h"
#include "dbn.h"
#include "ubm.h"

ClickModel* create_cm(int model_type) {
    ClickModel *cm_temp;
    switch (model_type) {
        case 0:{
            PBM pbm_model;
            cm_temp = &pbm_model;
            break;
        }
        case 1:{
            CCM ccm_model;
            cm_temp = &ccm_model;
            break;
        }
        case 2:{
            DBN dbn_model;
            cm_temp = &dbn_model;
            break;
        }
        case 3:{
            UBM ubm_model;
            cm_temp = &ubm_model;
            break;
        }
        default: {
            PBM pbm_is_default;
            cm_temp = &pbm_is_default;
            break;
        }
    }
    return cm_temp->clone();
}

ParallelClickModel::ParallelClickModel(ClickModel* cm, int num_threads) {
    this->original_base_model = cm->clone();
    this->num_threads = num_threads;
    this->synchronizer = cm->get_synchronizer(num_threads);

    for (int i=0; i < num_threads; i++){
        ClickModel* temp_model = cm->clone();
        this->thread_models.push_back(temp_model);

        std::vector<ParamContainer*> temp_container_vec_new;
        std::vector<ParamContainer*> temp_container_vec_origin;
        temp_model->get_shared_containers(temp_container_vec_new, temp_container_vec_origin);
        this->synchronizer->add_thread_params(temp_container_vec_new, temp_container_vec_origin);
    }
}

void ParallelClickModel::init_query_dependent_parameters(const Dataset& dataset, Partition** partition) {
    for (int i=0; i < this->num_threads; i++){
        this->thread_models[i]->init_query_dependent_parameters(
                dataset, (*partition)->get_pr_part(i).thread_tasks(true));
    }
}

ParallelClickModel::~ParallelClickModel() {
    for (auto instance: this->thread_models){
        delete instance;
    }
    this->thread_models.clear();

    delete this->synchronizer;
    this->synchronizer = nullptr;
}

ClickModel *ParallelClickModel::get_thread_click_model(int thread_id) {
    return this->thread_models[thread_id];
}

Synchronizer *ParallelClickModel::fetch_synchronizer() {
    return this->synchronizer;
}
