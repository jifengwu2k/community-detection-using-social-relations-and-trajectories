All source code of "Community Detection using Social Relations and Trajectories".

## Requirements

- Unix-like system with the `/tmp` directory
- At least 8 GB of RAM
- clang++, supporting C++17, invokable with `clang++`
- The following C++ libraries installed:
  - Boost C++ Libraries
  - Eigen
  - [ben-strasser/fast-cpp-csv-parser](https://github.com/ben-strasser/fast-cpp-csv-parser)
  - [p-ranav/argparse](https://github.com/p-ranav/argparse)
  - [pybind/pybind11](https://github.com/pybind/pybind11)
- Python 3, invokable with `python3`, with the following extensions:
  - folium
  - haversine
  - matplotlib
  - more-itertools
  - networkx
  - notebook (JupyterLab FAILS to display pyecharts visualizations!)
  - numpy
  - pandas
  - pybind11
  - pyecharts
  - scipy

## Replication Instructions

NOTE: if you need to change experiment parameters, edit `parameters.sh`.

### Extract Social Networks and Trajectories

```
unzip social_networks.zip
unzip trajectories.zip
```

### Build Tools

Run `bash build_tools.sh`.

### Run Experiments

Run `bash run_experiments.sh`. This process takes a long time (around a day).

### Data Analysis

Run the following Jupyter Notebooks:

1. `case_study.ipynb`
2. `average_trajectory_similarity_calculation_time.ipynb`
3. `spatial_temporal_distance_correlations.ipynb`
4. `m_values_to_sizes.ipynb`
5. `m_values_to_pairwise_distances.ipynb`
6. `m_values_to_pairwise_similarities.ipynb`
7. `community_detection_times.ipynb`

