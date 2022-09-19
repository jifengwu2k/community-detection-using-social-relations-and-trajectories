#include <chrono>
#include <iomanip>
#include <fstream>
#include <string>
#include <tuple>
#include <vector>

#include <argparse/argparse.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include "argsort.hpp"
#include "enumerate.hpp"
#include "find_closest_matches.hpp"
#include "haversine.hpp"
#include "load_trajectory_dataset.hpp"
#include "profile.hpp"
#include "read_adjacency_list.hpp"
#include "trajectory.h"
#include "trajectory_similarity.hpp"
#include "stlc.hpp"
#include "spatiotemporal_lcss.hpp"
#include "write_vector.hpp"


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
typedef boost::graph_traits<Graph>::edge_iterator EdgeIterator;


void parse_command_line_arguments(
    int argc,
    const char** argv,
    std::string& input_graph_path,
    std::string& input_trajectories_path,
    std::string& output_path
) {
    // To start parsing command-line arguments, create an ArgumentParser
    argparse::ArgumentParser parser("");
    
    // Datatypes of arguments are strings.
    // For other datatypes, please provide a default value of the appropriate type. 
    
    // Optional arguments start with - or --, e.g., --verbose or -a.
    // Optional arguments can be placed anywhere in the input sequence.
    
    // Note that by using .default_value(false), if the optional argument isn’t used, it's value is automatically set to false.
    // By using .implicit_value(true), the user specifies that this option is more of a flag than something that requires a value. When the user provides the --verbose option, its value is set to true.
    parser.add_argument("-g", "--graph")
        .required()
        .help("specify the input graph (an adjacency list)");
    
    parser.add_argument("-t", "--trajectories")
        .required()
        .help(
            "specify the input trajectories (a CSV file with the columns user, latitude, longitude, timestamp)"
        );
    
    parser.add_argument("-o", "--output")
        .required()
        .help("specify the output file");
    
    // Parse arguments
    try {
        parser.parse_args(argc, argv);
    }
    catch (const std::runtime_error& e) {
        std::cerr << e.what() << '\n';
        // std::cout << program prints a help message, including the program usage and information about the arguments registered with the ArgumentParser.
        std::cerr << parser;
        exit(EXIT_FAILURE);
    }
    
    // Use arguments
    input_graph_path = parser.get<std::string>("--graph");
    input_trajectories_path = parser.get<std::string>("--trajectories");
    output_path = parser.get<std::string>("--output");
}


int main(int argc, const char* argv[]) {
    // parse command line arguments
    std::string input_graph_path;
    std::string input_trajectory_path;
    std::string output_path;
    
    parse_command_line_arguments(
        argc,
        argv,
        input_graph_path,
        input_trajectory_path,
        output_path
    );
    
    // load social network
    Graph social_network;
    boost::unordered_map<std::string, VertexDescriptor> string_to_vertex_descriptor_map;
    
    std::ifstream input_file_stream(input_graph_path);
    read_adjacency_list<boost::vertex_name_t>(
        social_network,
        string_to_vertex_descriptor_map,
        input_file_stream
    );
    
    const auto get_vertex_name = [&social_network](const VertexDescriptor& vertex_descriptor) {
        return boost::get(boost::vertex_name_t(), social_network, vertex_descriptor);
    };
    
    VertexIterator vertex_begin, vertex_end;
    std::tie(vertex_begin, vertex_end) = boost::vertices(social_network);

    EdgeIterator edge_begin, edge_end;
    std::tie(edge_begin, edge_end) = boost::edges(social_network);
    
    std::vector<EdgeDescriptor> edges(edge_begin, edge_end);
    
    // load trajectory_dataset
    boost::unordered_map<VertexDescriptor, Trajectory> trajectory_dataset;
    
    load_trajectory_dataset(
        string_to_vertex_descriptor_map,
        input_trajectory_path,
        trajectory_dataset
    );
  
    // initialize lengths_of_trajectories
    std::vector<size_t> lengths_of_trajectories;
    std::transform(
        vertex_begin,
        vertex_end,
        std::back_inserter(lengths_of_trajectories),
        [&trajectory_dataset](const VertexDescriptor& v) {
             return trajectory_dataset.at(v).size();
        }
    );
    
    // initialize overall_similarity_runtimes_microseconds
    double overall_similarity_max_similarity = 0;
    std::vector<time_t> overall_similarity_runtimes_microseconds = profile<std::chrono::microseconds>(
        [
            &social_network,
            &edge_begin,
            &edge_end,
            &trajectory_dataset,
            &overall_similarity_max_similarity
        ]() {
            for (EdgeIterator edge_iterator = edge_begin; edge_iterator != edge_end; ++edge_iterator) {
                const EdgeDescriptor& edge_descriptor = *edge_iterator;

                const VertexDescriptor& source = boost::source(edge_descriptor, social_network);
                const VertexDescriptor& target = boost::target(edge_descriptor, social_network);

                double similarity = trajectory_similarity(
                    trajectory_dataset.at(source),
                    trajectory_dataset.at(target),
                    1000,
                    3600
                );

                overall_similarity_max_similarity = std::max(overall_similarity_max_similarity, similarity);
            }
        }
    );
    std::cout << "overall_similarity_max_similarity: " << overall_similarity_max_similarity << '\n';

    // initialize spatiotemporal_lcss_runtimes_microseconds
    double spatiotemporal_lcss_max_similarity = 0;
    std::vector<time_t> spatiotemporal_lcss_runtimes_microseconds = profile<std::chrono::microseconds>(
        [
            &social_network,
            &edge_begin,
            &edge_end,
            &trajectory_dataset,
            &spatiotemporal_lcss_max_similarity
        ]() {
            for (EdgeIterator edge_iterator = edge_begin; edge_iterator != edge_end; ++edge_iterator) {
                const EdgeDescriptor& edge_descriptor = *edge_iterator;

                const VertexDescriptor& source = boost::source(edge_descriptor, social_network);
                const VertexDescriptor& target = boost::target(edge_descriptor, social_network);

                double similarity = spatiotemporal_lcss(
                    trajectory_dataset.at(source),
                    trajectory_dataset.at(target),
                    5000,
                    18000
                );

                spatiotemporal_lcss_max_similarity = std::max(spatiotemporal_lcss_max_similarity, similarity);
            }
        }
    );
    std::cout << "spatiotemporal_lcss_max_similarity: " << spatiotemporal_lcss_max_similarity << '\n';

    // initialize stlc_runtimes_microseconds
    double stlc_max_similarity = 0;
    std::vector<time_t> stlc_runtimes_microseconds = profile<std::chrono::microseconds>(
        [
            &social_network,
            &edge_begin,
            &edge_end,
            &trajectory_dataset,
            &stlc_max_similarity
        ]() {
            for (EdgeIterator edge_iterator = edge_begin; edge_iterator != edge_end; ++edge_iterator) {
                const EdgeDescriptor& edge_descriptor = *edge_iterator;

                const VertexDescriptor& source = boost::source(edge_descriptor, social_network);
                const VertexDescriptor& target = boost::target(edge_descriptor, social_network);

                double similarity = stlc(
                    trajectory_dataset.at(source),
                    trajectory_dataset.at(target),
                    0.5
                );

                stlc_max_similarity = std::max(stlc_max_similarity, similarity);
            }
        }
    );
    std::cout << "stlc_max_similarity: " << stlc_max_similarity << '\n';

    // write output_path
    std::ofstream output_file_stream(output_path);
    output_file_stream
        << '{'
        << std::quoted("lengths_of_trajectories") << ':' << lengths_of_trajectories << ','
        << std::quoted("number_of_calculations") << ':' << boost::num_edges(social_network) << ','
        << std::quoted("overall_similarity_runtimes_microseconds") << ':' << overall_similarity_runtimes_microseconds << ','
        << std::quoted("spatiotemporal_lcss_runtimes_microseconds") << ':' << spatiotemporal_lcss_runtimes_microseconds << ','
        << std::quoted("stlc_runtimes_microseconds") << ':' << stlc_runtimes_microseconds
        << '}'
        << '\n';

    return 0;
}
