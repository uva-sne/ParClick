//
// Created by Pooya Khandel on 15/04/2021.
//

#include "parallel_em.h"


void em_thread(int which_thread, const int& n_itr, Logging& logger,
               ClickModel* cm,
               Synchronizer* sync,
               Dataset& pr_dataset,
               ThreadPartition& tasks,
               std::map<int, std::array<double, 2>>& llh_task,
               std::map<int, Perplexity>& ppl_task,
               Barrier& br1,
               Barrier& br2
               )
{
    auto em_all_start_time = std::chrono::high_resolution_clock::now();

    cm->init_temporary_containers();

    std::vector<double> log_avg_itr;
    std::vector<double> log_comp_itr;
    std::vector<double> log_sync_itr;
    std::vector<double> log_updating_itr;
    double elapsed_timing{0.0};
    // Training Loop

    for (int itr = 0; itr < n_itr; itr++)
    {
        auto em_avg_itr_start_time = std::chrono::high_resolution_clock::now();
        cm->init_iteration_containers();  // Create new containers ....
        for (auto idx: tasks.thread_tasks(true)) {
            SearchSession sss = pr_dataset.get_search_session(idx);
            cm->process_session(sss);
        }

        auto em_update_start_time = std::chrono::high_resolution_clock::now();
        // Update private pars
        cm->update_thread_specific_parameters();

        auto em_sync_start_time = std::chrono::high_resolution_clock::now();
        // first wait for all to finish
        br1.Sync();  // read examinations
        sync->first_sync(which_thread);

        auto em_sync_middle_end_time = std::chrono::high_resolution_clock::now();

        // second wait for all to read
        br2.Sync();  // write examinations
        sync->second_sync(which_thread);

        auto em_avg_itr_stop_time = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> em_avg_itr_elapsed_time = em_avg_itr_stop_time - em_avg_itr_start_time;
        log_avg_itr.push_back(em_avg_itr_elapsed_time.count());

        std::chrono::duration<double> em_comp_elapsed_time = em_update_start_time - em_avg_itr_start_time;
        log_comp_itr.push_back(em_comp_elapsed_time.count());
        std::chrono::duration<double> em_sync_elapsed_time = em_sync_middle_end_time - em_sync_start_time;
        log_sync_itr.push_back(em_sync_elapsed_time.count());

        std::chrono::duration<double> em_updating_elapsed_time = em_avg_itr_stop_time - em_update_start_time;
        log_updating_itr.push_back(em_updating_elapsed_time.count() - em_sync_elapsed_time.count());
        if (which_thread == 0){
            std::cout << "Itr: " << itr << " Itr-time: " << em_avg_itr_elapsed_time.count() <<
            " Itr-EM_COMP: " << em_comp_elapsed_time.count() <<
            " Itr-EM_UPDATE: " << em_updating_elapsed_time.count() - em_sync_elapsed_time.count() <<
            " Itr-Sync: " << em_sync_elapsed_time.count() <<
            std::endl;
        }

        auto em_timing_stop_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> em_timing_elapsed_time = em_timing_stop_time - em_avg_itr_stop_time;
        elapsed_timing += em_timing_elapsed_time.count();
    }

    auto em_all_stop_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> em_all_elapsed_time = em_all_stop_time - em_all_start_time;
    logger.set_em_total_time(which_thread, em_all_elapsed_time.count() - elapsed_timing);

    double avg_itr_time = std::accumulate(log_avg_itr.begin(), log_avg_itr.end(), 0.0) / n_itr;
    logger.set_em_avg_time(which_thread, avg_itr_time);
    double avg_em_comp = std::accumulate(log_comp_itr.begin(), log_comp_itr.end(), 0.0) / n_itr;
    logger.set_em_computations_time(which_thread, avg_em_comp);
    double avg_em_sync = std::accumulate(log_sync_itr.begin(), log_sync_itr.end(), 0.0) / n_itr;
    logger.set_em_sync_time(which_thread, avg_em_sync);
    double avg_em_updating = std::accumulate(log_updating_itr.begin(), log_updating_itr.end(), 0.0) / n_itr;
    logger.set_em_updating_time(which_thread, avg_em_updating);

    // Evaluation
    auto llh_start_time = std::chrono::high_resolution_clock::now();

    LogLikelihood llh(cm);
    double llh_val = llh.evaluate(pr_dataset, tasks);
    int task_size = tasks.thread_tasks(false).size();
    std::array<double, 2> temp_arr{llh_val, static_cast<double>(task_size)};
    llh_task[which_thread] = temp_arr;

    auto llh_stop_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> llh_elapsed_time = llh_stop_time - llh_start_time;
    logger.set_llh_time(which_thread, llh_elapsed_time.count());

    Perplexity ppl;
    ppl.evaluate(cm, pr_dataset, tasks);
    ppl_task[which_thread] = ppl;
}

void em_parallel(ParallelClickModel& pcm, Dataset& pr_dataset,
                 Partition** pr_part, int n_threads, int n_itr)
{
    std::cout << "Expectation Maximization (EM) in parallel ..." << std::endl;

    // Creating Threads
    std::vector<std::thread> train_threads;
    train_threads.reserve(n_threads);

    // Init Barriers
    Barrier br1{static_cast<size_t>(n_threads)};
    Barrier br2{static_cast<size_t>(n_threads)};


    std::map<int, std::array<double, 2>> llh;
    std::map<int, Perplexity> ppl_map;

    Logging logger{n_threads};

    Synchronizer* sync = pcm.fetch_synchronizer();
    sync->say_hello();

    for (int t_id=0; t_id < n_threads; t_id++){
        train_threads.emplace_back(em_thread, t_id, std::ref(n_itr), std::ref(logger),
                                   pcm.get_thread_click_model(t_id),
                                   sync,
                                   std::ref(pr_dataset), std::ref((*pr_part)->get_pr_part(t_id)),
                                   std::ref(llh),
                                   std::ref(ppl_map),
                                   std::ref(br1),
                                   std::ref(br2)
                                   );
    }

    for (auto & thread : train_threads){
        thread.join();
    }

    std::cout << "end" << std::endl;

    double total_llh_sum = 0.0;
    double total_llh_sessions = 0.0;
    std::for_each(llh.begin(), llh.end(), [&] (std::pair<const int, std::array<double, 2>>& llh_task) {
        total_llh_sum += llh_task.second[0];
        total_llh_sessions += llh_task.second[1];
    });

    std::cout << "Total Log likelihood is: " << total_llh_sum / total_llh_sessions << std::endl;
    double total_task_size{0.0};
    std::array<double, 10> temp_task_rank_perplexities{0.0};
    //
    for (auto const& itr: ppl_map){
        total_task_size += itr.second.task_size;
        for (int j{0}; j < 10; j++){
            temp_task_rank_perplexities[j] += itr.second.task_rank_perplexities[j];
        }
    }

    std::array<double, 10> ppl_per_rank{};
    double ppl_value;
    for (int i{0}; i < 10; i++){
        ppl_per_rank[i] = std::pow(2, (-1 * temp_task_rank_perplexities[i])/total_task_size);
        std::cout << "Perplexity at rank " << i << " is: " << ppl_per_rank[i] << std::endl;
    }
    ppl_value = std::accumulate(ppl_per_rank.begin(), ppl_per_rank.end(), 0.0) / 10.0;
    std::cout << "Perplexity is: " << ppl_value << std::endl;

    std::cout << "Average Time per Iteration: " << logger.average_thread_time(logger.em_avg_iteration_time) <<
    "\nAverage time per computation in each Iteration: " << logger.average_thread_time(logger.em_computations_time) <<
    "\nAverage time per synchronization in each Iteration: " << logger.average_thread_time(logger.em_sync_time) <<
    "\nAverage time per updating in each Iteration: " << logger.average_thread_time(logger.em_updating_time) <<
    "\nTotal time of training: " << logger.average_thread_time(logger.em_total_iteration_time) <<
    "\nEvaluation time: " << logger.average_thread_time(logger.llh_time) << std::endl;

}
