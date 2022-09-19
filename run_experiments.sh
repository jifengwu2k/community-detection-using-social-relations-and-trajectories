# Parse arguments


EXPERIMENT_ROOT="$(dirname "$0")"


# Load parameters


source "$EXPERIMENT_ROOT/parameters.sh"


# Build executables


pushd "$EXPERIMENTAL_CODE_DIRECTORY"
make
popd


# Calculate the Spatiotemporal Distances of Matching Points within our trajectory similarity algorithm, OverallSimilarity.


mkdir -p "$MATCHING_POINT_SPATIAL_TEMPORAL_DISTANCES_DIRECTORY"

for social_network_path in "$SOCIAL_NETWORKS_DIRECTORY"/*
do
    social_network="$(basename "$social_network_path")"
    
    trajectory_path="$TRAJECTORIES_DIRECTORY/$social_network"
    
    echo "$MATCHING_POINT_SPATIAL_TEMPORAL_DISTANCE_PATH" -g "$social_network_path" -t "$trajectory_path" --delta "$DELTA" --tau "$TAU" -o "$MATCHING_POINT_SPATIAL_TEMPORAL_DISTANCES_DIRECTORY/$social_network"
    "$MATCHING_POINT_SPATIAL_TEMPORAL_DISTANCE_PATH" -g "$social_network_path" -t "$trajectory_path" --delta "$DELTA" --tau "$TAU" -o "$MATCHING_POINT_SPATIAL_TEMPORAL_DISTANCES_DIRECTORY/$social_network"
done


# Calculate the Spatiotemporal Distances of Matching Points within the Spatiotemporal LCSS algorithm.


mkdir -p "$SPATIOTEMPORAL_LCSS_MATCHING_POINT_SPATIAL_TEMPORAL_DISTANCES_DIRECTORY"

for social_network_path in "$SOCIAL_NETWORKS_DIRECTORY"/*
do
    social_network="$(basename "$social_network_path")"
    
    trajectory_path="$TRAJECTORIES_DIRECTORY/$social_network"
    
    mkdir -p "$SPATIOTEMPORAL_LCSS_MATCHING_POINT_SPATIAL_TEMPORAL_DISTANCES_DIRECTORY/$social_network"
    
    for factor in $(seq 1 1 5)
    do
        echo "$SPATIOTEMPORAL_LCSS_MATCHING_POINT_SPATIAL_TEMPORAL_DISTANCE_PATH" -g "$social_network_path" -t "$trajectory_path" --epsilon "$(expr "$DELTA" '*' "$factor")" --delta "$(expr "$TAU" '*' "$factor")" -o "$SPATIOTEMPORAL_LCSS_MATCHING_POINT_SPATIAL_TEMPORAL_DISTANCES_DIRECTORY/$social_network/$factor"
        "$SPATIOTEMPORAL_LCSS_MATCHING_POINT_SPATIAL_TEMPORAL_DISTANCE_PATH" -g "$social_network_path" -t "$trajectory_path" --epsilon "$(expr "$DELTA" '*' "$factor")" --delta "$(expr "$TAU" '*' "$factor")" -o "$SPATIOTEMPORAL_LCSS_MATCHING_POINT_SPATIAL_TEMPORAL_DISTANCES_DIRECTORY/$social_network/$factor"
    done
done


# Calculate the Spatiotemporal Distances of Matching Points within the STLC algorithm.


mkdir -p "$STLC_MATCHING_POINT_SPATIAL_TEMPORAL_DISTANCES_DIRECTORY"

for social_network_path in "$SOCIAL_NETWORKS_DIRECTORY"/*
do
    social_network="$(basename "$social_network_path")"
    
    trajectory_path="$TRAJECTORIES_DIRECTORY/$social_network"
    
    mkdir -p "$STLC_MATCHING_POINT_SPATIAL_TEMPORAL_DISTANCES_DIRECTORY/$social_network"
    
    for lambda in $(seq 0.1 0.1 0.9)
    do
        echo "$STLC_MATCHING_POINT_SPATIAL_TEMPORAL_DISTANCE_PATH" -g "$social_network_path" -t "$trajectory_path" --lambda "$lambda" -o "$STLC_MATCHING_POINT_SPATIAL_TEMPORAL_DISTANCES_DIRECTORY/$social_network/$lambda"
        "$STLC_MATCHING_POINT_SPATIAL_TEMPORAL_DISTANCE_PATH" -g "$social_network_path" -t "$trajectory_path" --lambda "$lambda" -o "$STLC_MATCHING_POINT_SPATIAL_TEMPORAL_DISTANCES_DIRECTORY/$social_network/$lambda"
    done
done


# Profile the runtimes of our trajectory similarity algorithm, OverallSimilarity, as well as Spatiotemporal LCSS and STLC.


mkdir -p "$TRAJECTORY_SIMILARITY_RUNTIMES_DIRECTORY"

for social_network_path in "$SOCIAL_NETWORKS_DIRECTORY"/*
do
    social_network="$(basename "$social_network_path")"
    
    trajectory_path="$TRAJECTORIES_DIRECTORY/$social_network"
    
    echo "$PROFILE_TRAJECTORY_SIMILARITY_RUNTIMES_PATH" -g "$social_network_path" -t "$trajectory_path" -o "$TRAJECTORY_SIMILARITY_RUNTIMES_DIRECTORY/$social_network"
    "$PROFILE_TRAJECTORY_SIMILARITY_RUNTIMES_PATH" -g "$social_network_path" -t "$trajectory_path" -o "$TRAJECTORY_SIMILARITY_RUNTIMES_DIRECTORY/$social_network"
done


# Run community detection using our trajectory similarity algorithm, OverallSimilarity, and our community detection algorithm.


for social_network_path in "$SOCIAL_NETWORKS_DIRECTORY"/*
do
    social_network="$(basename "$social_network_path")"
    
    trajectory_path="$TRAJECTORIES_DIRECTORY/$social_network"
    
    for k in $K_VALUES
    do
        mkdir -p "$DETECTED_COMMUNITIES_DIRECTORY/$social_network/$k"
        mkdir -p "$COMMUNITY_DETECTION_TIMES_DIRECTORY/$social_network/$k"
        
        for m in $M_VALUES
        do
            echo "$COMMUNITY_DETECTION_PATH" -k "$k" -m "$m" -g "$social_network_path" -t "$trajectory_path" -o "$DETECTED_COMMUNITIES_DIRECTORY/$social_network/$k/$m"
            "$COMMUNITY_DETECTION_PATH" -k "$k" -m "$m" -g "$social_network_path" -t "$trajectory_path" -o "$DETECTED_COMMUNITIES_DIRECTORY/$social_network/$k/$m" > "$COMMUNITY_DETECTION_TIMES_DIRECTORY/$social_network/$k/$m"
        done
    done
done


# Calculate the k-cores of the social networks.


for social_network_path in "$SOCIAL_NETWORKS_DIRECTORY"/*
do
    social_network="$(basename "$social_network_path")"
    
    mkdir -p "$K_CORES_DIRECTORY/$social_network"
    
    for k in $K_VALUES
    do
        echo "$CALCULATE_K_CORE_PATH" -k "$k" -g "$social_network_path" -o "$K_CORES_DIRECTORY/$social_network/$k"
        "$CALCULATE_K_CORE_PATH" -k "$k" -g "$social_network_path" -o "$K_CORES_DIRECTORY/$social_network/$k"
    done
done


# Calculate all pairwise trajectory similarities using our trajectory similarity algorithm, OverallSimilarity, among the users of the social networks.


min_k="$(echo "$K_VALUES" | sort -n | head -1)"

mkdir -p "$PAIRWISE_SIMILARITIES_DIRECTORY"

for social_network_path in "$SOCIAL_NETWORKS_DIRECTORY"/*
do
    social_network="$(basename "$social_network_path")"
    
    trajectory_path="$TRAJECTORIES_DIRECTORY/$social_network"
    
    echo "$CALCULATE_PAIRWISE_SIMILARITIES_PATH" -g "$K_CORES_DIRECTORY/$social_network/$min_k" -t "$trajectory_path" -o "$PAIRWISE_SIMILARITIES_DIRECTORY/$social_network"
    "$CALCULATE_PAIRWISE_SIMILARITIES_PATH" -g "$K_CORES_DIRECTORY/$social_network/$min_k" -t "$trajectory_path" -o "$PAIRWISE_SIMILARITIES_DIRECTORY/$social_network"
done

