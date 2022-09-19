#include <iomanip>
#include <fstream>
#include <string>
#include <vector>

#include <argparse/argparse.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include "always_true_predicate.hpp"
#include "calculate_core_number.hpp"
#include "does_vertex_descriptor_coreness_satisfy_requirement.hpp"
#include "read_adjacency_list.hpp"
#include "write_edge_list.hpp"


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
    unsigned int& k,
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
    
    // There are scenarios where you would like to make an optional argument required.
    // If the user does not provide a value for this parameter, an exception is thrown.
    parser.add_argument("-g", "--graph")
        .required()
        .help("specify the input graph (an adjacency list)");

    parser.add_argument("-k", "--k")
        .required()
        .scan<'u', unsigned int>()
        .help("specify the value of k");
    
    parser.add_argument("-o", "--output")
        .required()
        .help("specify the output file (an adjacency list)");
    
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
    k = parser.get<unsigned int>("--k");
    output_path = parser.get<std::string>("--output");
}


int main(int argc, const char* argv[]) {
    // parse command line arguments
    std::string input_graph_path;
    unsigned int k;
    std::string output_path;
    
    parse_command_line_arguments(
        argc,
        argv,
        input_graph_path,
        k,
        output_path
    );
    
    // load input graph
    Graph graph;
    boost::unordered_map<std::string, VertexDescriptor> string_to_vertex_descriptor_map;
    
    std::ifstream input_file_stream(input_graph_path);
    read_adjacency_list<boost::vertex_name_t>(
        graph,
        string_to_vertex_descriptor_map,
        input_file_stream
    );
    
    // calculate core number
    boost::unordered_map<VertexDescriptor, DegreeSizeType> core_number = calculate_core_number(graph);

    // create graph_filtered_with_edge_predicate_and_vertex_predicate
    AlwaysTruePredicate edge_predicate;

    DoesVertexDescriptorCorenessSatisfyRequirement<decltype(core_number), DegreeSizeType> vertex_predicate(
        &core_number,
        k
    );

    boost::filtered_graph<Graph, decltype(edge_predicate), decltype(vertex_predicate)> graph_filtered_with_edge_predicate_and_vertex_predicate(
        graph,
        edge_predicate,
        vertex_predicate
    );

    // write graph_filtered_with_edge_predicate_and_vertex_predicate
    std::ofstream output_file_stream(output_path);
    write_edge_list<boost::vertex_name_t>(
        graph_filtered_with_edge_predicate_and_vertex_predicate,
        output_file_stream
    );
    
    return 0;
}

