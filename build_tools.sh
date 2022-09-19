# Parse arguments


EXPERIMENT_ROOT="$(dirname "$0")"


# Load parameters


source "$EXPERIMENT_ROOT/parameters.sh"


# Build tools


pushd "$EXPERIMENTAL_CODE_DIRECTORY"
make
popd

clang++ -std=c++11 -fPIC -shared -O3 -Wall -undefined dynamic_lookup $(python3 -m pybind11 --includes) graph_distance.cpp -o graph_distance$(python3-config --extension-suffix)

