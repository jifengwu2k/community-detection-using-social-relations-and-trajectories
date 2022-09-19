#ifndef SPATIOTEMPORAL_LCSS_HPP
#define SPATIOTEMPORAL_LCSS_HPP

#include <stddef.h>

#include <algorithm>
#include <limits>
#include <tuple>
#include <utility>

#include <boost/unordered_set.hpp>
#include <eigen3/Eigen/Core>

#include "haversine.hpp"
#include "point.h"
#include "trajectory.h"


double spatiotemporal_lcss(
    const Trajectory& first_trajectory,
    const Trajectory& second_trajectory,
    const double epsilon,
    const double delta
) {
    const auto spatial_distance = [](const Point& first, const Point& second) {
        return haversine(
            first.latitude,
            first.longitude,
            second.latitude,
            second.longitude
        );
    };

    const auto temporal_distance = [](const Point& first, const Point& second) {
        return (first.timestamp > second.timestamp) ?
            (first.timestamp - second.timestamp) :
            (second.timestamp - first.timestamp);
    };

    const auto is_same_point = [epsilon, delta, &spatial_distance, &temporal_distance](const Point& first, const Point& second) {
        return (
            (temporal_distance(first, second) < delta) && (spatial_distance(first, second) < epsilon) 
        );
    };

    const size_t first_trajectory_length = first_trajectory.size(), second_trajectory_length = second_trajectory.size();
    
    Eigen::Array<size_t, Eigen::Dynamic, Eigen::Dynamic> common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(
        first_trajectory_length + 1,
        second_trajectory_length + 1
    );
    
    size_t i, j;

    for (i = 0; i <= first_trajectory_length; ++i) {
        common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(i, 0) = 0;
    }

    for (j = 0; j <= second_trajectory_length; ++j) {
        common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(0, j) = 0;
    }

    for (j = 1; j <= second_trajectory_length; ++j) {
        for (i = 1; i <= first_trajectory_length; ++i) {
            // match
            if (is_same_point(first_trajectory[i - 1], second_trajectory[j - 1])) {
                common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(i, j) = common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(i - 1, j - 1) + 1;
            }
            else {
                if (common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(i, j - 1) > common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(i - 1, j)) {
                    common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(i, j) = common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(i, j - 1);
                }
                else {
                    common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(i, j) = common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(i - 1, j);
                }
            }
        }
    }

    size_t common_points = common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(first_trajectory_length, second_trajectory_length);
    
    return (double)common_points / (double)std::min(first_trajectory_length, second_trajectory_length);
}


template <typename MatchingPointCallback> double spatiotemporal_lcss(
    const Trajectory& first_trajectory,
    const Trajectory& second_trajectory,
    const double epsilon,
    const double delta,
    const MatchingPointCallback& matching_point_callback
) {
    const auto spatial_distance = [](const Point& first, const Point& second) {
        return haversine(
            first.latitude,
            first.longitude,
            second.latitude,
            second.longitude
        );
    };

    const auto temporal_distance = [](const Point& first, const Point& second) {
        return (first.timestamp > second.timestamp) ?
            (first.timestamp - second.timestamp) :
            (second.timestamp - first.timestamp);
    };

    const auto is_same_point = [epsilon, delta, &spatial_distance, &temporal_distance](const Point& first, const Point& second) {
        return (
            (temporal_distance(first, second) < delta) && (spatial_distance(first, second) < epsilon) 
        );
    };

    const size_t first_trajectory_length = first_trajectory.size(), second_trajectory_length = second_trajectory.size();
    
    Eigen::Array<size_t, Eigen::Dynamic, Eigen::Dynamic> common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(
        first_trajectory_length + 1,
        second_trajectory_length + 1
    );
    
    size_t i, j;

    for (i = 0; i <= first_trajectory_length; ++i) {
        common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(i, 0) = 0;
    }

    for (j = 0; j <= second_trajectory_length; ++j) {
        common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(0, j) = 0;
    }

    boost::unordered_set<std::pair<size_t, size_t>> is_same_point_indices;

    for (j = 1; j <= second_trajectory_length; ++j) {
        for (i = 1; i <= first_trajectory_length; ++i) {
            // match
            if (is_same_point(first_trajectory[i - 1], second_trajectory[j - 1])) {
                common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(i, j) = common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(i - 1, j - 1) + 1;
                
                is_same_point_indices.emplace(i - 1, j - 1);
            }
            else {
                if (common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(i, j - 1) > common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(i - 1, j)) {
                    common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(i, j) = common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(i, j - 1);
                }
                else {
                    common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(i, j) = common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(i - 1, j);
                }
            }
        }
    }

    i = first_trajectory_length, j = second_trajectory_length;
    while ((i > 0) && (j > 0)) {
        if (is_same_point_indices.count({i - 1, j - 1})) {
            matching_point_callback(first_trajectory[i - 1], second_trajectory[j - 1]);
            --i;
            --j;
            continue;
        }
        else {
            if (common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(i, j - 1) > common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(i - 1, j)) {
                --j;
                continue;
            }
            else {
                --i;
                continue;
            }
        }
    }

    size_t common_points = common_points_between_first_i_points_of_first_trajectory_and_first_j_points_of_second_trajectory(first_trajectory_length, second_trajectory_length);
    
    return (double)common_points / (double)std::min(first_trajectory_length, second_trajectory_length);
}

#endif
