#ifndef READ_ADJACENCY_LIST_HPP
#define READ_ADJACENCY_LIST_HPP

/**
 * بسم الله الرحمن الرحيم
 * 
 * File: read_adjacency_list.hpp
 * Author: Abbas Wu
 * Date of Creation: 2022/04/18
 * 
 * Copyright (C) 2022 Abbas Wu.
 * Distributed under the MIT License.
 * 
 * Reads an Adjacency List into a boost::graph object.
 * Stores vertex names as std::string objects in the vertex property specified with the template type PropertyTag.
 * 
 * Adjacency List format requirements:
 * 1. Each line must contain elements (representing vertices) separated by separators (satisfying std::isspace() or std::ispunct()).
 * 2. All edges can be written as vertex pairs.
 * 3. It's possible to write a vertex's all connections on the same line.
 * The example below represents a graph with 3 edges ("a" -> "b", "b" -> "c", and "b" -> "d"):
 * a;b
 * b;c;d
 * 
 * Usage example:
 * #include <iostream>
 * #include <string>
 * #include <boost/property_map/property_map.hpp>
 * #include <boost/graph/adjacency_list.hpp>
 * typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::property<boost::vertex_name_t, std::string>> Graph;
 * Graph graph;
 * read_adjacency_list<boost::vertex_name_t>(graph, std::cin);
 */

// References
// https://gephi.org/users/supported-graph-formats/csv-format/
// https://www.boost.org/doc/libs/1_70_0/libs/tokenizer/doc/tokenizer.htm
// http://www.cplusplus.com/forum/general/224845/
// property map header file
// https://valelab4.ucsf.edu/svn/3rdpartypublic/boost/libs/graph/doc/property_map.html
// predefined tags
// https://cs.brown.edu/~jwicks/boost/libs/graph/doc/using_adjacency_list.html#sec:adjacency-list-properties
// return the property map associated with a graph for a property
// return the property value associated with a vertex or edge for a property
// set the property value associated with a vertex or edge for a property
// https://cs.brown.edu/~jwicks/boost/libs/graph/doc/PropertyGraph.html
// example
// https://stackoverflow.com/questions/53550797/how-do-you-access-edge-properties-in-the-boost-graph-library
// https://stackoverflow.com/questions/67110765/using-boost-how-can-i-put-get-custom-edge-properties-as-a-struct

#include <iostream>
#include <string>

#include <boost/tokenizer.hpp>
#include <boost/graph/graph_traits.hpp>


template <typename Graph, typename StringToVertexDescriptorMap, typename PropertyMap> inline typename boost::graph_traits<Graph>::vertex_descriptor& get_or_insert_vertex(
    Graph& graph,
    StringToVertexDescriptorMap& string_to_vertex_descriptor_map,
    PropertyMap& property_map,
    const std::string& name
) {
    // If the name is not in the StringToVertexDescriptorMap
    // Add the vertex to the Graph
    // Add a mapping between its string and its vertex descriptor to the StringToVertexDescriptorMap
    // Set the property associated with the vertex to its string
    // And return the vertex descriptor
    // Else, return its vertex descriptor from the StringToVertexDescriptorMap
    if (string_to_vertex_descriptor_map.count(name) == 0) {
    	typename boost::graph_traits<Graph>::vertex_descriptor& vertex_descriptor = (string_to_vertex_descriptor_map[name] = boost::add_vertex(graph));
    	boost::put(property_map, vertex_descriptor, name);
    	return vertex_descriptor;
    } else return string_to_vertex_descriptor_map.at(name);
}

template <typename PropertyTag, typename Graph, typename StringToVertexDescriptorMap> void read_adjacency_list(
    Graph& graph,
    StringToVertexDescriptorMap& string_to_vertex_descriptor_map,
    std::istream& input_stream
) {
    // Get the property map for the property specified by the PropertyTag type    
    typename boost::property_map<Graph, PropertyTag>::type property_map = boost::get(
        PropertyTag(),
        graph
    );
    
    // Read stream line by line
    std::string line;
    while (std::getline(input_stream, line)) {
        // Split lines into tokens
        // No parsing is actually done upon construction
        // Parsing is done on demand as the tokens are accessed via the iterator
        boost::tokenizer<> tokens(line);
        
        // Iterate tokens
        boost::tokenizer<>::iterator it = tokens.begin(), end = tokens.end();
        if (it != end) {
            // Contains first token, which is the edges' tail
            const std::string& edges_tail_string = *it;
            typename boost::graph_traits<Graph>::vertex_descriptor edges_tail = get_or_insert_vertex(
                graph,
                string_to_vertex_descriptor_map,
                property_map,
                edges_tail_string
            );
            
            // Iterate the remaining tokens, which are the edges' heads
            for (++it; it != end; ++it) {
                const std::string& edge_head_string = *it;
                typename boost::graph_traits<Graph>::vertex_descriptor edge_head = get_or_insert_vertex(
                    graph,
                    string_to_vertex_descriptor_map,
                    property_map,
                    edge_head_string
                );
                
                // Add the edge
                boost::add_edge(edges_tail, edge_head, graph);
            }
        }
    }
}

template <typename PropertyTag, typename Graph> void read_adjacency_list(
    Graph& graph,
    std::istream& input_stream
) {
    std::unordered_map<std::string, typename boost::graph_traits<Graph>::vertex_descriptor> string_to_vertex_descriptor_map;
    return read_adjacency_list<PropertyTag>(
        graph,
        string_to_vertex_descriptor_map,
        input_stream
    );
}

#endif

