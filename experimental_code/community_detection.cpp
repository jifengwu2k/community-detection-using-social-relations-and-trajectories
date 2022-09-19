// install the following c++ package
// https://github.com/p-ranav/argparse
// compile with -std=c++17

#include <chrono>
#include <iomanip>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <argparse/argparse.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/copy.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include "trajectory.h"
#include "calculate_core_number.hpp"
#include "calculate_filtered_edge_set.hpp"
#include "does_vertex_descriptor_coreness_satisfy_requirement.hpp"
#include "is_edge_descriptor_in_edge_set.hpp"
#include "load_trajectory_dataset.hpp"
#include "profile.hpp"
#include "read_adjacency_list.hpp"
#include "trajectory_similarity.hpp"
#include "write_edge_list.hpp"
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
typedef boost::graph_traits<Graph>::degree_size_type DegreeSizeType;


void parse_command_line_arguments(
    int argc,
    const char** argv,
    std::string& input_graph_path,
    std::string& input_trajectories_path,
    unsigned int& k,
    unsigned int& m,
    double& tau,
    double& delta,
    std::string& output_graph_path
) {
    // To start parsing command-line arguments, create an ArgumentParser
    argparse::ArgumentParser parser("");
    
    // Datatypes of arguments are strings.
    // For other datatypes, please provide a default value of the appropriate type. 
    
    // Optional arguments start with - or --, e.g., --verbose or -a.
    // Optional arguments can be placed anywhere in the input sequence.
    
    // There are scenarios where you would like to make an optional argument required.
    // If the user does not provide a value for this parameter, an exception is thrown.
    parser.add_argument("-g", "--graph")
        .required()
        .help("specify the input graph (an adjacency list)");
    
    parser.add_argument("-t", "--trajectories")
        .required()
        .help(
            "specify the input trajectories (a CSV file with the columns user, latitude, longitude, timestamp)"
        );
    
    parser.add_argument("-k", "--k")
        .required()
        .scan<'u', unsigned int>()
        .help("specify the coreness requirement for each vertex");

    parser.add_argument("-m", "--m")
        .required()
        .scan<'u', unsigned int>()
        .help("specify the number of mutual nearest neighbors to consider for each vertex");
    
    // Alternatively, you could provide a default value
    parser.add_argument("--delta")
        .required()
        .scan<'g', double>()
        .default_value<double>(1000)
        .help(
            "the parameter delta (spatial time constant, in meters)"
        );
    
    parser.add_argument("--tau")
        .required()
        .scan<'g', double>()
        .default_value<double>(3600)
        .help(
            "the parameter tau (temporal time constant, in seconds)"
        );
    
    parser.add_argument("-o", "--output")
        .required()
        .help("specify the output graph (an adjacency list)");
    
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
    k = parser.get<unsigned int>("--k");
    m = parser.get<unsigned int>("--m");
    tau = parser.get<double>("--tau");
    delta = parser.get<double>("--delta");
    output_graph_path = parser.get<std::string>("--output");
}    


int main(int argc, const char* argv[]) {
    // parse command line arguments
    std::string input_graph_path;
    std::string input_trajectories_path;
    unsigned int k;
    unsigned int m;
    double tau;
    double delta;
    std::string output_graph_path;
    
    parse_command_line_arguments(
        argc,
        argv,
        input_graph_path,
        input_trajectories_path,
        k,
        m,
        tau,
        delta,
        output_graph_path
    );
    
    // create calculate_trajectory_similarity
    const auto calculate_trajectory_similarity = [&tau, &delta](
        const Trajectory& first,
        const Trajectory& second
    ) {
        return trajectory_similarity(first, second, tau, delta);
    };
    
    // load social_network
    Graph social_network;
    boost::unordered_map<std::string, VertexDescriptor> string_to_vertex_descriptor_map;
    
    std::ifstream input_file_stream(input_graph_path);
    read_adjacency_list<boost::vertex_name_t>(
        social_network,
        string_to_vertex_descriptor_map,
        input_file_stream
    );
    
    // load trajectory_dataset
    boost::unordered_map<VertexDescriptor, Trajectory> trajectory_dataset;
    load_trajectory_dataset(
        string_to_vertex_descriptor_map,
        input_trajectories_path,
        trajectory_dataset
    );

    boost::unordered_set<EdgeDescriptor> filtered_edge_set;
    IsEdgeDescriptorInEdgeSet<decltype(filtered_edge_set)> edge_predicate;
    std::unique_ptr<boost::filtered_graph<Graph, decltype(edge_predicate)>> social_network_filtered_with_edge_predicate;
    boost::unordered_map<VertexDescriptor, DegreeSizeType> core_number;
    DoesVertexDescriptorCorenessSatisfyRequirement<decltype(core_number), DegreeSizeType> vertex_predicate;
    std::unique_ptr<boost::filtered_graph<Graph, decltype(edge_predicate), decltype(vertex_predicate)>> social_network_filtered_with_edge_predicate_and_vertex_predicate;

    std::vector<time_t> community_detection_runtimes_microseconds = profile<std::chrono::microseconds>(
        [
            &k,
            &m,
            &social_network,
            &trajectory_dataset,
            &calculate_trajectory_similarity,
            &filtered_edge_set,
            &edge_predicate,
            &social_network_filtered_with_edge_predicate,
            &core_number,
            &vertex_predicate,
            &social_network_filtered_with_edge_predicate_and_vertex_predicate
        ]() {
            filtered_edge_set = calculate_filtered_edge_set(
                social_network,
                trajectory_dataset,
                calculate_trajectory_similarity,
                m
            );

            // create social_network_filtered_with_edge_predicate
            edge_predicate = IsEdgeDescriptorInEdgeSet<decltype(filtered_edge_set)>(
                &filtered_edge_set
            );

            social_network_filtered_with_edge_predicate = std::make_unique<boost::filtered_graph<Graph, decltype(edge_predicate)>>(
                social_network,
                edge_predicate
            );
            
            // calculate core_number
            core_number = calculate_core_number(*social_network_filtered_with_edge_predicate);

            // create social_network_filtered_with_edge_predicate_and_vertex_predicate
            vertex_predicate = DoesVertexDescriptorCorenessSatisfyRequirement<decltype(core_number), DegreeSizeType>(
                &core_number,
                k
            );

            social_network_filtered_with_edge_predicate_and_vertex_predicate = std::make_unique<boost::filtered_graph<Graph, decltype(edge_predicate), decltype(vertex_predicate)>>(
                social_network,
                edge_predicate,
                vertex_predicate
            );
        }
    );

    // write community_detection_runtimes_microseconds
    std::cout << community_detection_runtimes_microseconds << '\n';

    // write social_network_filtered_with_edge_predicate_and_vertex_predicate
    std::ofstream output_file_stream(output_graph_path);
    write_edge_list<boost::vertex_name_t>(
        *social_network_filtered_with_edge_predicate_and_vertex_predicate,
        output_file_stream
    );
    
    return 0;
}
