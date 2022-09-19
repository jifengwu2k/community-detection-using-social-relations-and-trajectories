#ifndef FIND_CLOSEST_MATCHES_HPP
#define FIND_CLOSEST_MATCHES_HPP

#include "move_forward_to_find_next_minima.hpp"
#include "point.h"
#include "trajectory.h"


template <typename Consumer> void find_closest_matches(
    const Trajectory& from,
    const Trajectory& to,
    const Consumer& consumer
) {
    Trajectory::const_iterator source_iterator = from.cbegin();
    const Trajectory::const_iterator source_end = from.cend();
    
    if (source_iterator == source_end) return;
    
    for (
        Trajectory::const_iterator target_iterator = to.cbegin();
        target_iterator != to.cend();
        ++target_iterator
    ) {
        source_iterator = move_forward_to_find_next_minima(
            source_iterator,
            source_end,
            [target_iterator](const Trajectory::const_iterator t_source_iterator) {
                return (t_source_iterator->timestamp > target_iterator->timestamp) ?
                    (t_source_iterator->timestamp - target_iterator->timestamp) :
                    (target_iterator->timestamp - t_source_iterator->timestamp);
            }
        );
        
        consumer(source_iterator, target_iterator);
    }
}

#endif

