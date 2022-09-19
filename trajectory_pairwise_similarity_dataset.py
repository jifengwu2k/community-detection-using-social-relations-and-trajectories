import numpy as np


class TrajectoryPairwiseSimilarityDataset:
    __slots__ = ('vertices_to_indices', 'pairwise_similarities')
    def __init__(self, vertex_set, trajectory_similarity_filepath):
        n = len(vertex_set)

        self.vertices_to_indices = vertices_to_indices = {
            vertex: i
            for i, vertex in enumerate(vertex_set)
        }

        self.pairwise_similarities = pairwise_similarities = np.empty(
            n * (n - 1) // 2,
            dtype=float
        )

        with open(trajectory_similarity_filepath, 'r') as fp:
            lines = iter(fp)
            next(lines)

            for line in lines:
                first_vertex, second_vertex, pairwise_similarity = line.split(',')
                if first_vertex in vertices_to_indices and second_vertex in vertices_to_indices:
                    index_in_pairwise_similarities = self.index_in_pairwise_similarities(first_vertex, second_vertex)
                    pairwise_similarities[index_in_pairwise_similarities] = float(pairwise_similarity)

    def index_in_pairwise_similarities(self, first_vertex, second_vertex):
        first_vertex_index = self.vertices_to_indices[first_vertex]
        second_vertex_index = self.vertices_to_indices[second_vertex]

        smaller_index = min(first_vertex_index, second_vertex_index)
        larger_index = max(first_vertex_index, second_vertex_index)
        
        return smaller_index * (2 * len(self.vertices_to_indices) - smaller_index - 1) // 2 + (larger_index - smaller_index - 1)

    def pairwise_similarity(self, first_vertex, second_vertex):
        return self.pairwise_similarities[self.index_in_pairwise_similarities(first_vertex, second_vertex)]
