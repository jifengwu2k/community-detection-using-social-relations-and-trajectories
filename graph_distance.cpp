#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>

#include "experimental_code/read_adjacency_list.hpp"
#include "experimental_code/sizes_to_offsets.hpp"


// Graph typedefs
typedef boost::adjacency_list<
    boost::vecS,
    boost::vecS,
    boost::undirectedS,
    boost::property<boost::vertex_name_t, std::string>
> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor VertexDescriptor;
typedef boost::graph_traits<Graph>::edge_descriptor EdgeDescriptor;
typedef boost::graph_traits<Graph>::vertex_iterator VertexIterator;
typedef boost::graph_traits<Graph>::adjacency_iterator AdjacencyIterator;


struct GraphDistance {
    Graph graph;
    boost::unordered_map<std::string, VertexDescriptor> string_to_vertex_descriptor_map;
    boost::unordered_map<VertexDescriptor, size_t> vertex_descriptor_to_connected_component_index_map;
    std::vector<size_t> size_of_connected_components;
    boost::unordered_map<VertexDescriptor, size_t> vertex_descriptor_to_index_map;
    std::vector<size_t> pairwise_index_offsets_of_connected_components;
    std::vector<double> pairwise_distances;

    GraphDistance(
        const std::string& input_graph_path
    ) {
        // load graph and string_to_vertex_descriptor_map
        std::ifstream input_file_stream(input_graph_path);
        read_adjacency_list<boost::vertex_name_t>(
            graph,
            string_to_vertex_descriptor_map,
            input_file_stream
        );

        // initialize vertex_descriptor_to_connected_component_index_map
        const size_t number_of_connected_components = boost::connected_components(graph, boost::make_assoc_property_map(vertex_descriptor_to_connected_component_index_map));

        // initialize size_of_connected_components, vertex_descriptor_to_index_map
        size_of_connected_components.resize(
            number_of_connected_components,
            0
        );

        VertexIterator vertex_iterator, vertex_end;
        for (
            std::tie(vertex_iterator, vertex_end) = boost::vertices(graph);
            vertex_iterator != vertex_end;
            ++vertex_iterator
        ) {
            const VertexDescriptor& vertex_descriptor = *vertex_iterator;
            const size_t connected_component_index = vertex_descriptor_to_connected_component_index_map.at(vertex_descriptor);

            vertex_descriptor_to_index_map[vertex_descriptor] = size_of_connected_components.at(connected_component_index);
            ++size_of_connected_components.at(connected_component_index);
        }

        // initialize number_of_pairs_in_connected_components
        std::vector<size_t> number_of_pairs_in_connected_components;
        std::transform(
            size_of_connected_components.cbegin(),
            size_of_connected_components.cend(),
            std::back_inserter(number_of_pairs_in_connected_components),
            [](const size_t number) { return number * (number - 1) / 2; }
        );

        // initialize pairwise_index_offsets_of_connected_components
        pairwise_index_offsets_of_connected_components = sizes_to_offsets(number_of_pairs_in_connected_components);

        // allocate space for pairwise_distances
        pairwise_distances.resize(
            std::accumulate(number_of_pairs_in_connected_components.cbegin(), number_of_pairs_in_connected_components.cend(), 0),
            std::numeric_limits<double>::infinity()
        );

        // initialize thread_pool
	    boost::asio::thread_pool thread_pool(std::thread::hardware_concurrency());
        
        for (
            auto iterator = vertex_descriptor_to_index_map.cbegin();
            iterator != vertex_descriptor_to_index_map.cend();
            ++iterator
        ) {
            const VertexDescriptor& bfs_tree_root = iterator->first;
            const size_t& bfs_tree_root_index = iterator->second;
            const size_t& connected_component_index = vertex_descriptor_to_connected_component_index_map.at(bfs_tree_root);
            const size_t& connected_component_size = size_of_connected_components.at(connected_component_index);
            const size_t& pairwise_index_offset = pairwise_index_offsets_of_connected_components.at(connected_component_index);

            // https://networkx.org/documentation/networkx-1.10/_modules/networkx/algorithms/shortest_paths/unweighted.html#all_pairs_shortest_path_length
            // https://en.wikipedia.org/wiki/Breadth-first_search
            const auto bfs_task = [
                this,
                bfs_tree_root,
                bfs_tree_root_index,
                connected_component_size,
                pairwise_index_offset
            ]() {
                // label root as explored
                boost::unordered_set<VertexDescriptor> explored {bfs_tree_root};

                // Q.enqueue(root)
                std::vector<VertexDescriptor> current_layer {bfs_tree_root};
                unsigned long level = 0;

                // while Q is not empty do
                while (current_layer.size()) {
                    std::vector<VertexDescriptor> next_layer;

                    for (const VertexDescriptor& vertex_descriptor: current_layer) {
                        // v := Q.dequeue()

                        // 存bfs_tree_root到vertex_descriptor的距离
                        const size_t vertex_descriptor_index = vertex_descriptor_to_index_map.at(vertex_descriptor);

                        if (bfs_tree_root_index != vertex_descriptor_index) {
                            pairwise_distances.at(pairwise_index_offset + vertex_descriptor_index_to_pairwise_index(bfs_tree_root_index, vertex_descriptor_index, connected_component_size)) = level;
                        }

                        // for all edges from v to w in G.adjacentEdges(v) do
                        AdjacencyIterator adjacency_iterator, adjacency_end;
                        for (
                            std::tie(adjacency_iterator, adjacency_end) = boost::adjacent_vertices(vertex_descriptor, graph);
                            adjacency_iterator != adjacency_end;
                            ++adjacency_iterator
                        ) {
                            const VertexDescriptor& adjacency = *adjacency_iterator;

                            // if w is not labeled as explored then
                            if (!explored.count(adjacency)) {
                                // label w as explored
                                explored.insert(adjacency);

                                // Q.enqueue(w)
                                next_layer.push_back(adjacency);
                            }
                        }
                    }

                    current_layer = std::move(next_layer);
                    ++level;
                }
            };

            boost::asio::post(
                thread_pool,
                bfs_task
            );
        }

        // join thread_pool
        thread_pool.join();
    }

    inline size_t vertex_descriptor_index_to_pairwise_index(const size_t first_index, const size_t second_index, const size_t n) const {
        size_t smaller_index = std::min(first_index, second_index), larger_index = std::max(first_index, second_index);
        return smaller_index * (2 * n - smaller_index - 1) / 2 + larger_index - smaller_index - 1;
    }

    double operator()(const std::string& first_vertex, const std::string& second_vertex) const {
        const VertexDescriptor& first_vertex_descriptor = string_to_vertex_descriptor_map.at(first_vertex);
        const VertexDescriptor& second_vertex_descriptor = string_to_vertex_descriptor_map.at(second_vertex);

        const size_t& first_vertex_index = vertex_descriptor_to_index_map.at(first_vertex_descriptor);
        const size_t& second_vertex_index = vertex_descriptor_to_index_map.at(second_vertex_descriptor);

        const size_t& first_vertex_connected_component_index = vertex_descriptor_to_connected_component_index_map.at(first_vertex_descriptor);
        const size_t& second_vertex_connected_component_index = vertex_descriptor_to_connected_component_index_map.at(first_vertex_descriptor);

        if (first_vertex_connected_component_index == second_vertex_connected_component_index) {
            if (first_vertex_index != second_vertex_index) {
                const size_t& connected_component_size = size_of_connected_components.at(first_vertex_connected_component_index);
                const size_t& pairwise_index_offset = pairwise_index_offsets_of_connected_components.at(first_vertex_connected_component_index);

                return pairwise_distances.at(pairwise_index_offset + vertex_descriptor_index_to_pairwise_index(first_vertex_index, second_vertex_index, connected_component_size));
            }
            else {
                return 0;
            }
        }
        else {
            return std::numeric_limits<double>::infinity();
        }
    }
};


PYBIND11_MODULE(graph_distance, m) {
    // Bindings for class GraphDistance
    pybind11::class_<GraphDistance>(
        m,
        "GraphDistance",
        // Python supports an extremely general and convenient approach for exchanging data between plugin libraries.
        // Types can expose a buffer view, which provides fast direct access to the raw internal data representation.
        pybind11::buffer_protocol()
    )
        // bindings for constructor
        // pybind11::init<> internally uses C++11 brace initialization to call the constructor of the target class
        // This means that it can be used to bind implicit constructors as well
        .def(pybind11::init<const std::string&>())
        // bindings for instance fields
        // bindings for class methods with template parameters
        .def("__call__", &GraphDistance::operator())
        // The following binding code exposes the contents as a buffer object, making it possible to cast into NumPy arrays.
        // It is even possible to completely avoid copy operations with Python expressions like np.array(instance, copy=False).
        .def_buffer([](GraphDistance& graph_distance) -> pybind11::buffer_info {
            return pybind11::buffer_info(
                // Pointer to buffer
                graph_distance.pairwise_distances.data(),
                // Size of one scalar
                sizeof(double),
                // Python struct-style format descriptor
                pybind11::format_descriptor<double>::format(),
                // Number of dimensions
                1,
                // Buffer dimensions
                { graph_distance.pairwise_distances.size() },
                // Strides (in bytes) for each index
                { sizeof(double) }
            );
        });
}
