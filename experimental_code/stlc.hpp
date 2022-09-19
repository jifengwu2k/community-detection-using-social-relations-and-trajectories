#ifndef STLC_HPP
#define STLC_HPP

#include <math.h>

#include <tuple>

#include "haversine.hpp"
#include "min_element_and_value.hpp"
#include "point.h"
#include "trajectory.h"


template <typename PointDistance> double one_way_similarity(
    const Trajectory& from,
    const Trajectory& to,
    const PointDistance& point_distance
) {
    double sum = 0;

    for (const Point& source: from) {
        Trajectory::const_iterator pointer_to_matching_point;
        double distance;

        std::tie(pointer_to_matching_point, distance) = min_element_and_value(
            to.cbegin(),
            to.cend(),
            [&point_distance, &source](const Point& target) {
                return point_distance(source, target);
            }
        );

        sum += exp(-distance);
    }

    return sum / (double)(from.size());
}

template <typename PointDistance, typename MatchingPointCallback> double one_way_similarity(
    const Trajectory& from,
    const Trajectory& to,
    const PointDistance& point_distance,
    const MatchingPointCallback& matching_point_callback
) {
    double sum = 0;

    for (const Point& source: from) {
        Trajectory::const_iterator pointer_to_matching_point;
        double distance;

        std::tie(pointer_to_matching_point, distance) = min_element_and_value(
            to.cbegin(),
            to.cend(),
            [&point_distance, &source](const Point& target) {
                return point_distance(source, target);
            }
        );

        matching_point_callback(source, *pointer_to_matching_point);
        sum += exp(-distance);
    }

    return sum / (double)(from.size());
}


double spatial_similarity(
    const Trajectory& first,
    const Trajectory& second
) {
    const auto spatial_distance = [](const Point& first, const Point& second) {
        return haversine(
            first.latitude,
            first.longitude,
            second.latitude,
            second.longitude
        );
    };

    return one_way_similarity(first, second, spatial_distance) + one_way_similarity(second, first, spatial_distance);
}

template <typename MatchingPointCallback> double spatial_similarity(
    const Trajectory& first,
    const Trajectory& second,
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

    return one_way_similarity(first, second, spatial_distance, matching_point_callback) + one_way_similarity(second, first, spatial_distance, matching_point_callback);
}


double temporal_similarity(
    const Trajectory& first,
    const Trajectory& second
) {
    const auto temporal_distance = [](const Point& first, const Point& second) {
        return (first.timestamp > second.timestamp) ?
            (first.timestamp - second.timestamp) :
            (second.timestamp - first.timestamp);
    };

    return one_way_similarity(first, second, temporal_distance) + one_way_similarity(second, first, temporal_distance);
}

template <typename MatchingPointCallback> double temporal_similarity(
    const Trajectory& first,
    const Trajectory& second,
    const MatchingPointCallback& matching_point_callback
) {
    const auto temporal_distance = [](const Point& first, const Point& second) {
        return (first.timestamp > second.timestamp) ?
            (first.timestamp - second.timestamp) :
            (second.timestamp - first.timestamp);
    };

    return one_way_similarity(first, second, temporal_distance, matching_point_callback) + one_way_similarity(second, first, temporal_distance, matching_point_callback);
}


double stlc(
    const Trajectory& first,
    const Trajectory& second,
    const double lambda
) {
    return lambda * spatial_similarity(first, second) + (1 - lambda) * temporal_similarity(second, first);
}

template <typename MatchingPointCallback> double stlc(
    const Trajectory& first,
    const Trajectory& second,
    const double lambda,
    const MatchingPointCallback& matching_point_callback
) {
    return lambda * spatial_similarity(first, second, matching_point_callback) + (1 - lambda) * temporal_similarity(second, first, matching_point_callback);
}

#endif
