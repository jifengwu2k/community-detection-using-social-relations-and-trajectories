#ifndef PROFILE_HPP
#define PROFILE_HPP

#include <chrono>
#include <vector>


template <typename ToDuration, typename Callable> std::vector<time_t> profile(
    const Callable& callable,
    const size_t number_of_times = 8
) {
    std::vector<time_t> results(number_of_times);
    for (size_t i = 0; i < number_of_times; ++i) {
        auto start = std::chrono::high_resolution_clock::now();

        callable();
        
        auto stop = std::chrono::high_resolution_clock::now();
        
        results[i] = std::chrono::duration_cast<ToDuration>(stop - start).count();
    }
    return results;
}

#endif
