cmake_minimum_required(VERSION 2.8)
project(parclick)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-std=c++11 -O3 -pthread")

include_directories(.)
add_executable(parclick main.cpp data/dataset.cpp click_models/param.cpp data/partition.cpp parallel_em/barrier.cpp data/search.cpp parallel_em/parallel_em.cpp parallel_em/parallel_em.h click_models/base.cpp click_models/base.cpp click_models/base.h parallel_em/synchronization.cpp parallel_em/synchronization.h utils/logging.cpp utils/logging.h click_models/pbm.cpp click_models/pbm.h click_models/ccm.cpp click_models/ccm.h click_models/factor.cpp click_models/factor.h click_models/evaluation.cpp click_models/evaluation.h click_models/dbn.cpp click_models/dbn.h click_models/ubm.cpp click_models/ubm.h)