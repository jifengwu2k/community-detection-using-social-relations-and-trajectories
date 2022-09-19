#ifndef CALCULATE_CORE_NUMBER_HPP
#define CALCULATE_CORE_NUMBER_HPP

#include <algorithm>
#include <limits>
#include <list>
#include <numeric>
#include <tuple>
#include <vector>

#include <boost/graph/graph_traits.hpp>
#include <boost/unordered_map.hpp>


template <typename Graph> boost::unordered_map<
    typename boost::graph_traits<Graph>::vertex_descriptor,
    typename boost::graph_traits<Graph>::degree_size_type
> calculate_core_number(
    const Graph& graph
) {
    boost::unordered_map<
        typename boost::graph_traits<Graph>::vertex_descriptor,
        typename boost::graph_traits<Graph>::degree_size_type
    > vertex_descriptor_to_coreness_map;

    std::vector<typename boost::graph_traits<Graph>::vertex_descriptor> vertex_descriptors_in_ascending_order_of_coreness(boost::num_vertices(graph));

    std::vector<size_t> start_indices_of_corenesses(boost::num_vertices(graph), 0);
    
    boost::unordered_map<
        typename boost::graph_traits<Graph>::vertex_descriptor,
        size_t
    > vertex_descriptor_to_index_map;

    // initialize each node's coreness to its degree 

    auto vertices_begin_and_end = boost::vertices(graph);
    auto vertex_iterator = vertices_begin_and_end.first, vertex_end = vertices_begin_and_end.second;

    for (
        ;
        vertex_iterator != vertex_end;
        ++vertex_iterator
    ) {
        typename boost::graph_traits<Graph>::degree_size_type degree = boost::degree(
            *vertex_iterator,
            graph
        );

        vertex_descriptor_to_coreness_map[*vertex_iterator] = degree;
        ++start_indices_of_corenesses[degree];
    }

    std::partial_sum(
        start_indices_of_corenesses.begin(),
        start_indices_of_corenesses.end(),
        start_indices_of_corenesses.begin()
    );

    for (
        std::tie(vertex_iterator, vertex_end) = boost::vertices(graph);
        vertex_iterator != vertex_end;
        ++vertex_iterator
    ) {
        typename boost::graph_traits<Graph>::degree_size_type degree = boost::degree(
            *vertex_iterator,
            graph
        );

        size_t index = --start_indices_of_corenesses[degree];
        vertex_descriptors_in_ascending_order_of_coreness[index] = *vertex_iterator;
        vertex_descriptor_to_index_map[*vertex_iterator] = index;
    }

    // iteratively update coreness

    for (size_t vertex_index = 0; vertex_index < vertex_descriptors_in_ascending_order_of_coreness.size(); ++vertex_index) {
        typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor = vertex_descriptors_in_ascending_order_of_coreness[vertex_index];
        typename boost::graph_traits<Graph>::degree_size_type vertex_coreness = vertex_descriptor_to_coreness_map[vertex_descriptor];

        auto adjacency_begin_and_end = boost::adjacent_vertices(vertex_descriptor, graph);
        auto adjacency_iterator = adjacency_begin_and_end.first, adjacency_end = adjacency_begin_and_end.second;
        for (; adjacency_iterator != adjacency_end; ++adjacency_iterator) {
            typename boost::graph_traits<Graph>::vertex_descriptor adjacent_vertex_descriptor = *adjacency_iterator;
            typename boost::graph_traits<Graph>::degree_size_type adjacent_vertex_coreness = vertex_descriptor_to_coreness_map[adjacent_vertex_descriptor];

            if (adjacent_vertex_coreness > vertex_coreness) {
                size_t adjacent_vertex_index = vertex_descriptor_to_index_map[adjacent_vertex_descriptor];

                size_t first_vertex_with_adjacent_vertex_coreness_index = start_indices_of_corenesses[adjacent_vertex_coreness];
                typename boost::graph_traits<Graph>::vertex_descriptor first_vertex_with_adjacent_vertex_coreness_descriptor = vertex_descriptors_in_ascending_order_of_coreness[first_vertex_with_adjacent_vertex_coreness_index];

                if (adjacent_vertex_descriptor != first_vertex_with_adjacent_vertex_coreness_descriptor) {
                    std::swap(
                        vertex_descriptors_in_ascending_order_of_coreness[adjacent_vertex_index],
                        vertex_descriptors_in_ascending_order_of_coreness[first_vertex_with_adjacent_vertex_coreness_index]
                    );

                    std::swap(
                        vertex_descriptor_to_index_map[adjacent_vertex_descriptor],
                        vertex_descriptor_to_index_map[first_vertex_with_adjacent_vertex_coreness_descriptor]
                    );
                }

                ++start_indices_of_corenesses[adjacent_vertex_coreness];
                --vertex_descriptor_to_coreness_map[adjacent_vertex_descriptor];
            }
        }
    }

    return vertex_descriptor_to_coreness_map;
}

#endif
