#ifndef TRAJECTORY_SIMILARITY_HPP
#define TRAJECTORY_SIMILARITY_HPP

#include <time.h>

#include <tuple>

#include "find_closest_matches.hpp"
#include "haversine.hpp"
#include "pairwise_adaptor.hpp"
#include "trajectory.h"


template <typename PointSimilarity> double one_way_trajectory_similarity(
    const Trajectory& from,
    const Trajectory& to,
    const PointSimilarity& point_similarity
) {
    double total_time = 0;
    double total_area = 0;

    const auto update_total_time_total_area = [
        &total_time,
        &total_area
    ](
        const std::tuple<double, time_t>& t_old,
        const std::tuple<double, time_t>& t_new
    ) {
        double time_delta = std::get<1>(t_new) - std::get<1>(t_old);
        double area_delta = (std::get<0>(t_new) + std::get<0>(t_old)) * time_delta / 2;
        // std::cout << time_delta << ' ' << area_delta << '\n';
        total_time += time_delta;
        total_area += area_delta;    
    };

    PairwiseAdaptor<
        decltype(update_total_time_total_area),
        std::tuple<double, time_t>
    > update_total_time_total_area_adaptor(
        update_total_time_total_area
    );

    const auto closest_match_consumer = [
        &update_total_time_total_area_adaptor,
        &point_similarity
    ](
        const Trajectory::const_iterator source_iterator,
        const Trajectory::const_iterator target_iterator
    ) {
        // std::cout << '(' << source_iterator->latitude << ',' << source_iterator->longitude << ',' << source_iterator->timestamp << ')' << ' ' << '(' << target_iterator->latitude << ',' << target_iterator->longitude << ',' << target_iterator->timestamp << ')' << '\n';
        double similarity = point_similarity(*source_iterator, *target_iterator);
        // std::cout << similarity << '\n';
        update_total_time_total_area_adaptor(std::make_tuple(similarity, target_iterator->timestamp));
    };

    find_closest_matches(
        from,
        to,
        closest_match_consumer
    );

    return total_area / total_time;
}

double trajectory_similarity(
    const Trajectory& first,
    const Trajectory& second,
    const double delta,
    const double tau
) {
    const auto point_similarity = [
        &delta,
        &tau
    ](const Point& first, const Point& second) {
        double spatial_distance = haversine(
            first.latitude,
            first.longitude,
            second.latitude,
            second.longitude
        );
        
        double temporal_distance = (first.timestamp >= second.timestamp) ? (first.timestamp - second.timestamp) : (second.timestamp - first.timestamp);
        
        return exp((-spatial_distance / delta) + (-temporal_distance / tau));
    };

    return (one_way_trajectory_similarity(first, second, point_similarity) + one_way_trajectory_similarity(second, first, point_similarity)) / 2;
}

template <typename MatchingPointCallback> double trajectory_similarity(
    const Trajectory& first,
    const Trajectory& second,
    const double delta,
    const double tau,
    const MatchingPointCallback& matching_point_callback
) {
    const auto decorated_point_similarity = [
        &delta,
        &tau,
        &matching_point_callback
    ](const Point& first, const Point& second) {
        double spatial_distance = haversine(
            first.latitude,
            first.longitude,
            second.latitude,
            second.longitude
        );
        
        double temporal_distance = (first.timestamp >= second.timestamp) ? (first.timestamp - second.timestamp) : (second.timestamp - first.timestamp);
        
        double similarity = exp((-spatial_distance / delta) + (-temporal_distance / tau));

        matching_point_callback(first, second);

        return similarity;
    };

    return (one_way_trajectory_similarity(first, second, decorated_point_similarity) + one_way_trajectory_similarity(second, first, decorated_point_similarity)) / 2;
}

/*
#include <math.h>

#include <iostream>


double distance(const Point& first, const Point& second) {
    return sqrt((first.latitude - second.latitude) * (first.latitude - second.latitude) + (first.longitude - second.longitude) * (first.longitude - second.longitude));
}


int main() {
    Trajectory first {{47,34,-3}, {77,-67,0}, {21,28,2}, {-61,0,5}, {7,-133,7}, {53,176,11}, {13,148,13}};
    Trajectory second {{-6,59,1}, {-28,76,3}, {-74,57,6}, {62,-41,7}, {-74,16,9}};
    std::cout << trajectory_similarity(first, second, distance) << ' ' << trajectory_similarity(second, first, distance) << '\n';
    return 0;
}
*/

#endif

