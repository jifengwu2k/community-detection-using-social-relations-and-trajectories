#ifndef ARGSORT_HPP
#define ARGSORT_HPP

#include <algorithm>
#include <numeric>
#include <vector>


template <typename T> std::vector<size_t> argsort(const std::vector<T>& vector) {
    std::vector<size_t> sorted_indices(vector.size());
    std::iota(sorted_indices.begin(), sorted_indices.end(), 0);
    std::sort(sorted_indices.begin(), sorted_indices.end(), [&vector](const size_t& first, const size_t& second) {
        return vector[first] < vector[second];
    });
    return sorted_indices;
}

#endif

