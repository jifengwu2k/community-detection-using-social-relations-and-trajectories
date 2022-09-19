#ifndef LOAD_TRAJECTORY_DATASET_HPP
#define LOAD_TRAJECTORY_DATASET_HPP

#include <time.h>

#include <string>

#include <csv.h>


// load trajectory dataset in the form of a string to trajectory map
template <typename StringToTrajectoryMap> void load_trajectory_dataset(
    const std::string& trajectory_dataset_path,
    StringToTrajectoryMap& trajectory_dataset
) {
    io::CSVReader<4> in(trajectory_dataset_path);
    in.read_header(io::ignore_extra_column, "user", "latitude", "longitude", "timestamp");
    
    /* 每一行的字段 */
    std::string user;
    double latitude, longitude;
    time_t timestamp;
    
    /* 遍历各行 */
    while (in.read_row(user, latitude, longitude, timestamp)) {
        /* 存储相关信息 */
        trajectory_dataset[user].push_back({latitude, longitude, timestamp});
    }
}

// load trajectory dataset in the form of a vertex descriptor to trajectory map
template <typename StringToVertexDescriptorMap, typename VertexDescriptorToTrajectoryMap> void load_trajectory_dataset(
    const StringToVertexDescriptorMap& string_to_vertex_descriptor_map,
    const std::string& trajectory_dataset_path,
    VertexDescriptorToTrajectoryMap& trajectory_dataset
) {
    io::CSVReader<4> in(trajectory_dataset_path);
    in.read_header(io::ignore_extra_column, "user", "latitude", "longitude", "timestamp");
    
    /* 每一行的字段 */
    std::string user;
    double latitude, longitude;
    time_t timestamp;
    
    /* 遍历各行 */
    while (in.read_row(user, latitude, longitude, timestamp)) {
        /* 存储相关信息 */
        if (string_to_vertex_descriptor_map.count(user)) {
            trajectory_dataset[
                string_to_vertex_descriptor_map.at(user)
            ].push_back({latitude, longitude, timestamp});
        }
    }
}

#endif

