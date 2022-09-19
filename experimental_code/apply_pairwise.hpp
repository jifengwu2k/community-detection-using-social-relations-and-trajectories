#ifndef APPLY_PAIRWISE_HPP
#define APPLY_PAIRWISE_HPP

#include <math.h>

#include <tuple>
#include <iostream>


template <typename VectorLike> struct ApplyPairwise {
    const VectorLike& vector_like;
    const size_t exclusive_end_i;
    const size_t exclusive_end_pair_index;
    
    ApplyPairwise(const VectorLike& t_vector_like):
        vector_like(t_vector_like),
        exclusive_end_i(
            vector_like.size() ? vector_like.size() - 1 : 0
        ),
        exclusive_end_pair_index(
            (t_vector_like.size() > 1) ?
            (t_vector_like.size() * (t_vector_like.size() - 1) / 2) :
            0
        ) { }
    
    // if i < exclusive_end_i, returns a valid pair index
    // else, returns exclusive_end_pair_index
    size_t i_to_first_pair_index_in_row(const size_t t_i) const {
        if (t_i < exclusive_end_i) {
            return t_i * (2 * vector_like.size() - t_i - 1) / 2;
        }
        else {
            return exclusive_end_pair_index;
        }
    }
    
    // if x < exclusive_end_pair_index, returns a valid row index
    // else, returns exclusive_end_i
    size_t pair_index_to_i(const size_t t_pair_index) const {
        if (t_pair_index < exclusive_end_pair_index) {
            return ((2 * vector_like.size() - 1) - sqrt((2 * vector_like.size() - 1) * (2 * vector_like.size() - 1) - 8 * t_pair_index)) / 2;
        }
        else return exclusive_end_i;
    }
    
    // if x < exclusive_end_pair_index, returns a valid row index and column index pair
    // else, returns exclusive_end_i paired with 0
    std::pair<size_t, size_t> pair_index_to_i_and_j(const size_t t_pair_index) const {
        size_t i = pair_index_to_i(t_pair_index);
        if (i < exclusive_end_i) {
            size_t first_pair_index_in_row = i_to_first_pair_index_in_row(i);
            size_t j = i + (t_pair_index - first_pair_index_in_row) + 1;
            return {i, j};
        }
        else {
            return {i, 0};
        }
    }

    template <typename BinaryFunction> void operator()(const BinaryFunction& t_binary_function) const {
        for (size_t i = 0; i < vector_like.size(); ++i) {
            for (size_t j = i + 1; j < vector_like.size(); ++j) {
                t_binary_function(vector_like[i], vector_like[j]);
            }
        }
    }
    
    template <typename BinaryFunction> void operator() (
        const BinaryFunction& t_binary_function,
        const size_t t_inclusive_start_pair_index,
        const size_t t_exclusive_end_pair_index
    ) const {
        size_t inclusive_start_i, inclusive_start_j, inclusive_end_i, exclusive_end_j;
        std::tie(inclusive_start_i, inclusive_start_j) = pair_index_to_i_and_j(t_inclusive_start_pair_index);
        std::tie(inclusive_end_i, exclusive_end_j) = pair_index_to_i_and_j(t_exclusive_end_pair_index);
        
        // std::cout << inclusive_start_i << ' ' << inclusive_start_j << ' ' << inclusive_end_i << ' ' << exclusive_end_j << '\n';
        
        size_t i, j;
        for (
            i = inclusive_start_i, j = inclusive_start_j;
            i < inclusive_end_i;
            ++i, j = i + 1
        ) {
            for (
                ;
                j < vector_like.size();
                ++j
            ) {
                // std::cout << i << ' ' << j << '\n';
                t_binary_function(vector_like[i], vector_like[j]);
            }
        }
        
        if (i == inclusive_end_i) {
            for (
                ;
                j < exclusive_end_j;
                ++j
            ) {
                // std::cout << i << ' ' << j << '\n';
                t_binary_function(vector_like[i], vector_like[j]);
            }
        }
    }
};
    
#endif
