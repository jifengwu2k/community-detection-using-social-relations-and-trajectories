#ifndef SIZES_TO_OFFSETS_HPP
#define SIZES_TO_OFFSETS_HPP

#include <numeric>
#include <vector>


template <typename T> std::vector<T> sizes_to_offsets(const std::vector<T>& sizes) {
    std::vector<T> offsets;
    std::accumulate(sizes.cbegin(), sizes.cend(), 0, [&offsets](const T& accumulated_value, const T& next_value) {
        offsets.push_back(accumulated_value);
        return accumulated_value + next_value;
    });
    return offsets;
}

#endif
