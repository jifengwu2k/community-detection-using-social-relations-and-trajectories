#ifndef CALCULATE_FILTERED_EDGE_SET_HPP
#define CALCULATE_FILTERED_EDGE_SET_HPP

/**
 * https://stackoverflow.com/questions/66043842/what-is-the-runtime-complexity-of-boostedge-on-an-adjacency-list-with-vecs
 * https://www.boost.org/doc/libs/1_75_0/libs/graph/doc/using_adjacency_list.html
 * https://www.boost.org/doc/libs/1_39_0/libs/graph/doc/graph_concepts.html
 * https://stackoverflow.com/questions/4010097/general-use-cases-for-c-containers
 * https://en.cppreference.com/w/cpp/utility/tuple/ignore
 */

#include <algorithm>
#include <tuple>
#include <utility>
#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include <exception>
#include <iostream>


template <
    typename Graph,
    typename TrajectoryDataset,
    typename CalculateTrajectorySimilarity
> boost::unordered_set<typename boost::graph_traits<Graph>::edge_descriptor> calculate_filtered_edge_set(
    const Graph& social_network,
    const TrajectoryDataset& trajectory_dataset,
    const CalculateTrajectorySimilarity& calculate_trajectory_similarity,
    const size_t m
) {
    boost::unordered_set<
        typename boost::graph_traits<Graph>::edge_descriptor
    > selected, singly_checked;

    boost::unordered_map<
        typename boost::graph_traits<Graph>::edge_descriptor,
        double
    > similarities;
    
    typename boost::graph_traits<Graph>::vertex_iterator vertex_begin, vertex_end;
    std::tie(vertex_begin, vertex_end) = boost::vertices(social_network);

    boost::unordered_set<
        typename boost::graph_traits<Graph>::vertex_descriptor
    > remaining_vertices(vertex_begin, vertex_end);

    while (remaining_vertices.size()) {
        typename boost::graph_traits<Graph>::vertex_descriptor v = *(remaining_vertices.begin());
        remaining_vertices.erase(remaining_vertices.begin());

        boost::unordered_set<
            typename boost::graph_traits<Graph>::vertex_descriptor
        > current_layer { v };
        
        while (current_layer.size()) {
            boost::unordered_set<
                typename boost::graph_traits<Graph>::vertex_descriptor
            > next_layer;
            
            for (
                const typename boost::graph_traits<Graph>::vertex_descriptor& u: current_layer
            ) {
                std::vector<
                    std::pair<
                        double,
                        typename boost::graph_traits<Graph>::edge_descriptor
                    >
                > neighbors_and_similarities;
                
                typename boost::graph_traits<Graph>::out_edge_iterator out_edge_iterator, out_edge_end;
                for(
                    std::tie(out_edge_iterator, out_edge_end) = boost::out_edges(
                        u,
                        social_network
                    );
                    out_edge_iterator != out_edge_end;
                    ++out_edge_iterator
                ) {
                    const typename boost::graph_traits<Graph>::edge_descriptor& uv = *out_edge_iterator;
                    double trajectory_similarity;
                    
                    if (!similarities.count(uv)) {
                        const typename boost::graph_traits<Graph>::vertex_descriptor& v = boost::target(uv, social_network);
                        
                        try {
                            trajectory_similarity = calculate_trajectory_similarity(
                                trajectory_dataset.at(u),
                                trajectory_dataset.at(v)
                            );
                        }
                        catch (std::out_of_range& e) {
                            trajectory_similarity = 0;
                        }
                        
                        similarities[uv] = trajectory_similarity;
                    }
                    else {
                        trajectory_similarity = similarities[uv];
                    }
                    
                    neighbors_and_similarities.emplace_back(
                        trajectory_similarity,
                        uv
                    );
                }

                size_t m_ = std::min(
                    m,
                    neighbors_and_similarities.size()
                );
                
                std::partial_sort(
                    neighbors_and_similarities.begin(),
                    neighbors_and_similarities.begin() + m_,
                    neighbors_and_similarities.end(),
                    [](
                        const auto& first_pair,
                        const auto& second_pair
                    ) {
                        return first_pair.first > second_pair.first;
                    }
                );
                
                for (
                    size_t i = 0;
                    i < m_;
                    ++i
                ) {
                    const typename boost::graph_traits<Graph>::edge_descriptor& uv = neighbors_and_similarities[i].second;
                    
                    if (singly_checked.count(uv)) {
                        singly_checked.erase(uv);
                        selected.insert(uv);
                    }
                    else {
                        const typename boost::graph_traits<Graph>::vertex_descriptor& v = boost::target(uv, social_network);
                        
                        if (current_layer.count(v)) {
                            singly_checked.insert(uv);
                        }
                        else {
                            if (remaining_vertices.count(v)) {
                                singly_checked.insert(uv);
                                next_layer.insert(v);
                            }
                        }
                    }
                }
            }
            
            for (
                const typename boost::graph_traits<Graph>::vertex_descriptor& u: current_layer
            ) {
                remaining_vertices.erase(u);
            }
            
            current_layer = std::move(next_layer); 
        }
    }
    
    return selected;
}


template <
    typename Graph,
    typename VertexDescriptorToCorenessMap,
    typename TrajectoryDataset,
    typename CalculateTrajectorySimilarity
> boost::unordered_set<typename boost::graph_traits<Graph>::edge_descriptor> calculate_filtered_edge_set(
    const Graph& social_network,
    const VertexDescriptorToCorenessMap& vertex_descriptor_to_coreness_map,
    const TrajectoryDataset& trajectory_dataset,
    const CalculateTrajectorySimilarity& calculate_trajectory_similarity,
    const size_t k,
    const size_t m
) {
    boost::unordered_set<
        typename boost::graph_traits<Graph>::edge_descriptor
    > selected, singly_checked;

    boost::unordered_map<
        typename boost::graph_traits<Graph>::edge_descriptor,
        double
    > similarities;
    
    typename boost::graph_traits<Graph>::vertex_iterator vertex_begin, vertex_end;
    std::tie(vertex_begin, vertex_end) = boost::vertices(social_network);

    boost::unordered_set<
        typename boost::graph_traits<Graph>::vertex_descriptor
    > remaining_vertices;
    std::copy_if(
        vertex_begin,
        vertex_end,
        std::inserter(remaining_vertices, remaining_vertices.end()),
        [&vertex_descriptor_to_coreness_map, k](const typename boost::graph_traits<Graph>::vertex_descriptor& vertex_descriptor) {
            return vertex_descriptor_to_coreness_map.at(vertex_descriptor) > k;
        }
    );

    while (remaining_vertices.size()) {
        typename boost::graph_traits<Graph>::vertex_descriptor v = *(remaining_vertices.begin());
        remaining_vertices.erase(remaining_vertices.begin());

        boost::unordered_set<
            typename boost::graph_traits<Graph>::vertex_descriptor
        > current_layer { v };
        
        while (current_layer.size()) {
            boost::unordered_set<
                typename boost::graph_traits<Graph>::vertex_descriptor
            > next_layer;
            
            for (
                const typename boost::graph_traits<Graph>::vertex_descriptor& u: current_layer
            ) {
                std::vector<
                    std::pair<
                        double,
                        typename boost::graph_traits<Graph>::edge_descriptor
                    >
                > neighbors_and_similarities;
                
                typename boost::graph_traits<Graph>::out_edge_iterator out_edge_iterator, out_edge_end;
                for(
                    std::tie(out_edge_iterator, out_edge_end) = boost::out_edges(
                        u,
                        social_network
                    );
                    out_edge_iterator != out_edge_end;
                    ++out_edge_iterator
                ) {
                    const typename boost::graph_traits<Graph>::edge_descriptor& uv = *out_edge_iterator;
                    double trajectory_similarity;
                    
                    if (!similarities.count(uv)) {
                        const typename boost::graph_traits<Graph>::vertex_descriptor& v = boost::target(uv, social_network);
                        
                        try {
                            trajectory_similarity = calculate_trajectory_similarity(
                                trajectory_dataset.at(u),
                                trajectory_dataset.at(v)
                            );
                        }
                        catch (std::out_of_range& e) {
                            trajectory_similarity = 0;
                        }
                        
                        similarities[uv] = trajectory_similarity;
                    }
                    else {
                        trajectory_similarity = similarities[uv];
                    }
                    
                    neighbors_and_similarities.emplace_back(
                        trajectory_similarity,
                        uv
                    );
                }

                size_t m_ = std::min(
                    m,
                    neighbors_and_similarities.size()
                );
                
                std::partial_sort(
                    neighbors_and_similarities.begin(),
                    neighbors_and_similarities.begin() + m_,
                    neighbors_and_similarities.end(),
                    [](
                        const auto& first_pair,
                        const auto& second_pair
                    ) {
                        return first_pair.first > second_pair.first;
                    }
                );
                
                for (
                    size_t i = 0;
                    i < m_;
                    ++i
                ) {
                    const typename boost::graph_traits<Graph>::edge_descriptor& uv = neighbors_and_similarities[i].second;
                    
                    if (singly_checked.count(uv)) {
                        singly_checked.erase(uv);
                        selected.insert(uv);
                    }
                    else {
                        const typename boost::graph_traits<Graph>::vertex_descriptor& v = boost::target(uv, social_network);
                        
                        if (current_layer.count(v)) {
                            singly_checked.insert(uv);
                        }
                        else {
                            if (remaining_vertices.count(v)) {
                                singly_checked.insert(uv);
                                next_layer.insert(v);
                            }
                        }
                    }
                }
            }
            
            for (
                const typename boost::graph_traits<Graph>::vertex_descriptor& u: current_layer
            ) {
                remaining_vertices.erase(u);
            }
            
            current_layer = std::move(next_layer); 
        }
    }
    
    return selected;
}

#endif
