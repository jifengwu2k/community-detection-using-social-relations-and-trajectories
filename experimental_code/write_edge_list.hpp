#ifndef WRITE_EDGE_LIST_HPP
#define WRITE_EDGE_LIST_HPP

/**
 * بسم الله الرحمن الرحيم
 * 
 * File: write_edge_list.hpp
 * Author: Abbas Wu
 * Date of Creation: 2022/04/18
 * 
 * Copyright (C) 2022 Abbas Wu.
 * Distributed under the MIT License.
 * 
 * Writes a boost::graph object to an edge list.
 * Reads vertex names in the form of std::string objects in the vertex property specified with the template type PropertyTag.
 * 
 * Edge list format specification:
 * 1. Each line represents an edge.
 * 2. It contains 2 elements (representing vertices) separated by a space.
 * The example below represents a graph with 3 edges ("a" -> "b", "b" -> "c", and "b" -> "d"):
 * a b
 * b c
 * b d
 * 
 * Usage example:
 * #include <iostream>
 * #include <string>
 * #include <boost/property_map/property_map.hpp>
 * #include <boost/graph/adjacency_list.hpp>
 * typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::property<boost::vertex_name_t, std::string>> Graph;
 * Graph graph;
 * // modify the graph
 * write_edge_list<boost::vertex_name_t>(graph, std::cout);
 */

// References
// https://www.cplusplus.com/reference/fstream/ofstream/ofstream/
// boost/graph/graph_traits.hpp
// https://gephi.org/users/supported-graph-formats/csv-format/

#include <iostream>
#include <string>
#include <tuple>

#include <boost/property_map/property_map.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>


template <typename PropertyTag, typename Graph, typename EdgeIterator> void write_edge_list(
    const Graph& graph,
    const EdgeIterator edge_begin,
    const EdgeIterator edge_end,
    std::ostream& output_stream
) {
    // Get the property map for the property specified by the PropertyTag type    
    const auto property_map = boost::get(
        PropertyTag(),
        graph
    );
    
    const auto get_vertex_name = [&graph](const typename boost::graph_traits<Graph>::vertex_descriptor& vertex_descriptor) {
    	return boost::get(PropertyTag(), graph, vertex_descriptor);
    };
    
    // Iterate edges
    for (
        EdgeIterator edge_iterator = edge_begin;
        edge_iterator != edge_end;
        ++edge_iterator
    ) {
        const typename boost::graph_traits<Graph>::edge_descriptor& edge_descriptor = *edge_iterator;
        
        const typename boost::graph_traits<Graph>::vertex_descriptor& source_vertex_descriptor = boost::source(edge_descriptor, graph);
        const typename boost::graph_traits<Graph>::vertex_descriptor& target_vertex_descriptor = boost::target(edge_descriptor, graph);
        
        // Write edge to stream
        output_stream << get_vertex_name(source_vertex_descriptor) << ' ' << get_vertex_name(target_vertex_descriptor) << '\n';
    }
}

template <typename PropertyTag, typename Graph> void write_edge_list(
    const Graph& graph,
    std::ostream& output_stream
) {
    const auto edge_begin_edge_end_pair = boost::edges(graph);
    return write_edge_list<PropertyTag>(
        graph,
        edge_begin_edge_end_pair.first,
        edge_begin_edge_end_pair.second,
        output_stream
    );
}

#endif

