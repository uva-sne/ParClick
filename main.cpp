#include <iostream>
#include <string>
#include "data/dataset.h"
#include "click_models/base.h"
#include "parallel_em/parallel_em.h"


int main(int argc, char** argv){
    // Program is started
    std::map<int, std::string> supported_click_models{
            {0, "PBM"},
            {1, "CCM"},
            {2, "DBN"},
            {3, "UBM"}
    };

    auto start_time = std::chrono::high_resolution_clock::now();
    int n_threads{4};
    std::string raw_dataset_path{"YandexRelPredChallenge"};
    int n_iterations{50};
    int max_sessions{40000};
    int job_id{0};
    int partitioning_type{0};
    int model_type{3};
    bool filter_test{true};

    if (argc > 2){
        for (int i = 1; i < argc; ++i){
            if (std::string(argv[i]) == "--n-threads"){
                n_threads = std::stoi(argv[i+1]);
            }
            else if (std::string(argv[i]) == "--raw-path"){
                raw_dataset_path = argv[i+1];
            }
            else if (std::string(argv[i]) == "--itr"){
                n_iterations = std::stoi(argv[i+1]);
            }
            else if (std::string(argv[i]) == "--max-sessions"){
                max_sessions = std::stoi(argv[i+1]);
            }
            else if (std::string(argv[i]) == "--model-type"){
                model_type = std::stoi(argv[i+1]);
            }
            else if (std::string(argv[i]) == "--partition-type"){
                partitioning_type = std::stoi(argv[i+1]);
            }
            else if (std::string(argv[i]) == "--job-id"){
                job_id = std::stoi(argv[i+1]);
            }
        }
    }
    std::cout << "Job ID: " << job_id <<
    "\nNumber of threads: " << n_threads <<
    "\nRaw data path: " << raw_dataset_path <<
    "\nNumber of EM iterations: " << n_iterations <<
    "\nNumber of sessions: " << max_sessions <<
    "\nFilter unseen test queries: " << filter_test <<
    "\nPartitioning type: " << partitioning_type <<
    "\nModel type: " << supported_click_models[model_type] << std::endl;

    auto preprocess_start_time = std::chrono::high_resolution_clock::now();
    // parse dataset
    Dataset dataset;
    parse_dataset(dataset, raw_dataset_path, max_sessions);

    // Scatter data based on number of threads
    Partition* part;
    Partition base_part(n_threads, 0.2);
    MUPartition mu_part(n_threads, 0.2);
    if (partitioning_type == 0) {
        part = &base_part;
    }
    else if (partitioning_type == 1) {
        part = &mu_part;
    }
    else
        std::cout << "WRONG INPUT: PARTITION TYPE! " << std::endl;


    dataset.make_splits(n_threads, 0.2, filter_test, &part);

    /*----------------------------------------------------------------------*/
    ClickModel* cm = create_cm(model_type);
    cm->say_hello();
    ParallelClickModel pcm(cm, n_threads);
    pcm.init_query_dependent_parameters(dataset, &part);


    auto preprocess_stop_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_preprocess = preprocess_stop_time - preprocess_start_time;


    // Run the EM algorithm
    em_parallel(pcm, dataset, &part, n_threads, n_iterations);

    auto stop_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = stop_time - start_time;
    std::cout << "Finished!, Total Elapsed time: " << elapsed.count() << " seconds" << std::endl;
    std::cout << "PreProcess time: " << elapsed_preprocess.count() << std::endl << std::endl;

    std::cout << "Thread Workload-Train --- Thread No. Unique Queries" << std::endl;
    for (auto thread_partition: part->partition_map){
        std::cout << thread_partition.second.size() << "---" << thread_partition.second.size_queries(true) << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Thread Workload-Test --- Thread No. Unique Queries" << std::endl;
    for (auto thread_partition: part->partition_map){
        std::cout << thread_partition.second.size_test() << "---" << thread_partition.second.size_queries(false) << std::endl;
    }
    std::cout << std::endl;

    delete cm;
    return 0;
}