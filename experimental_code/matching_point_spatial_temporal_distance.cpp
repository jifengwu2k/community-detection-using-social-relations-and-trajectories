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
#include "read_adjacency_list.hpp"
#include "trajectory.h"
#include "trajectory_similarity.hpp"
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
    double& tau,
    double& delta,
    std::string& output_path
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
        .help("specify the output file (an CSV file with the columns first_user,second_user,spatial_distance,temporal_distance)");
    
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
    output_path = parser.get<std::string>("--output");
}


int main(int argc, const char* argv[]) {
    // parse command line arguments
    std::string input_graph_path;
    std::string input_trajectory_path;
    double tau;
    double delta;
    std::string output_path;
    
    parse_command_line_arguments(
        argc,
        argv,
        input_graph_path,
        input_trajectory_path,
        tau,
        delta,
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
  
    // write matching_point_spatial_temporal_distance
    std::ofstream output_file_stream(output_path);

    enumerate(
        edge_begin,
        edge_end,
        [
            &tau,
            &delta,
            &social_network,
            &get_vertex_name,
            &trajectory_dataset,
            &output_file_stream
        ](
            const size_t i,
            const EdgeDescriptor& edge_descriptor
        ) {
            const VertexDescriptor& source = boost::source(edge_descriptor, social_network);
            const VertexDescriptor& target = boost::target(edge_descriptor, social_network);
            
            const std::string& source_name = get_vertex_name(source);
            const std::string& target_name = get_vertex_name(target);

            std::vector<double> spatial_distances, temporal_distances;

            double similarity = trajectory_similarity(
                trajectory_dataset.at(source),
                trajectory_dataset.at(target),
                tau,
                delta,
                [&spatial_distances, &temporal_distances](const Point& source_point, const Point& target_point) {
                    const double spatial_distance = haversine(
                        source_point.latitude,
                        source_point.longitude,
                        target_point.latitude,
                        target_point.longitude
                    );
                    
                    const time_t temporal_distance = 
                        (source_point.timestamp > target_point.timestamp) ?
                        (source_point.timestamp - target_point.timestamp) :
                        (target_point.timestamp - source_point.timestamp)
                    ;

                    spatial_distances.push_back(spatial_distance);
                    temporal_distances.push_back(temporal_distance);
                }
            );

            output_file_stream
                << '{'
                << std::quoted("first_user") << ':' << source_name << ','
                << std::quoted("second_user") << ':' << target_name << ','
                << std::quoted("similarity") << ':' << similarity << ','
                << std::quoted("spatial_distances") << ':' << spatial_distances << ','
                << std::quoted("temporal_distances") << ':' << temporal_distances
                << '}'
                << '\n';
        }
    );

    return 0;
}

