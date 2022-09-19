all: matching_point_spatial_temporal_distance spatiotemporal_lcss_matching_point_spatial_temporal_distance stlc_matching_point_spatial_temporal_distance profile_trajectory_similarity_runtimes community_detection calculate_k_core calculate_pairwise_similarities

matching_point_spatial_temporal_distance: matching_point_spatial_temporal_distance.cpp
	clang++ -std=clang++17 -O3 matching_point_spatial_temporal_distance.cpp -o matching_point_spatial_temporal_distance -lpthread

spatiotemporal_lcss_matching_point_spatial_temporal_distance: spatiotemporal_lcss_matching_point_spatial_temporal_distance.cpp
	clang++ -std=clang++17 -O3 spatiotemporal_lcss_matching_point_spatial_temporal_distance.cpp -o spatiotemporal_lcss_matching_point_spatial_temporal_distance -lpthread

stlc_matching_point_spatial_temporal_distance: stlc_matching_point_spatial_temporal_distance.cpp
	clang++ -std=clang++17 -O3 stlc_matching_point_spatial_temporal_distance.cpp -o stlc_matching_point_spatial_temporal_distance -lpthread

profile_trajectory_similarity_runtimes: profile_trajectory_similarity_runtimes.cpp
	clang++ -std=clang++17 -O3 profile_trajectory_similarity_runtimes.cpp -o profile_trajectory_similarity_runtimes -lpthread

community_detection: community_detection.cpp
	clang++ -std=clang++17 -O3 community_detection.cpp -o community_detection -lpthread

calculate_k_core: calculate_k_core.cpp
	clang++ -std=clang++17 -O3 calculate_k_core.cpp -o calculate_k_core -lpthread

calculate_pairwise_similarities: calculate_pairwise_similarities.cpp
	clang++ -std=clang++17 -O3 calculate_pairwise_similarities.cpp -o calculate_pairwise_similarities -lpthread

