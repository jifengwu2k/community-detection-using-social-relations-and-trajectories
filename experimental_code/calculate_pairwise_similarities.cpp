// install the following c++ package
// https://github.com/p-ranav/argparse
// compile with -std=c++17

#include <fstream>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include <argparse/argparse.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/property_map/property_map.hpp>

#include "load_trajectory_dataset.hpp"
#include "read_adjacency_list.hpp"
#include "trajectory_similarity.hpp"


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
    double& tau,
    double& delta,
    std::string& output_pairwise_similarities_path
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
        .help("specify the output file (a CSV file with the columns first_user, second_user, similarity)");
    
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
    tau = parser.get<double>("--tau");
    delta = parser.get<double>("--delta");
    output_pairwise_similarities_path = parser.get<std::string>("--output");
}   


int main(int argc, const char* argv[]) {
    // parse command line arguments
    std::string input_graph_path;
    std::string input_trajectories_path;
    double tau;
    double delta;
    std::string output_pairwise_similarities_path;

    parse_command_line_arguments(
        argc,
        argv,
        input_graph_path,
        input_trajectories_path,
        tau,
        delta,
        output_pairwise_similarities_path
    );

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
    std::unordered_map<std::string, Trajectory> trajectory_dataset;

    load_trajectory_dataset(
        input_trajectories_path,
        trajectory_dataset
    );

    // calculate and write
    std::ofstream output_file_stream { output_pairwise_similarities_path };

    output_file_stream << "first_user,second_user,similarity" << '\n';

    for (
        auto first_iterator = string_to_vertex_descriptor_map.cbegin();
        first_iterator != string_to_vertex_descriptor_map.cend();
        ++first_iterator
    ) {
        auto second_iterator = first_iterator;

        for (
            ++second_iterator;
            second_iterator != string_to_vertex_descriptor_map.cend();
            ++second_iterator
        ) {
            const std::string& first_user = first_iterator->first;
            const std::string& second_user = second_iterator->first;
            const double similarity = trajectory_similarity(
                trajectory_dataset.at(first_user),
                trajectory_dataset.at(second_user),
                tau,
                delta
            );

            output_file_stream << first_user << ',' << second_user << ',' << similarity << '\n';
        }
    }

    return 0;
}
