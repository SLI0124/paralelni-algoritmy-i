#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <unordered_map>
#include <sstream>
#include <chrono>
#include <cmath>
#include <algorithm>

#define DAMPING_FACTOR 0.85
#define EPSILON 1e-6
#define MAX_ITERATIONS 100

/**
 * Structure to hold the adjacency list representation of the graph.
 */
struct AdjacencyList {
    std::unordered_map<int, std::vector<int> > n_minus; ///< Map of nodes to their incoming neighbors.
    std::unordered_map<int, std::vector<int> > n_plus; ///< Map of nodes to their outgoing neighbors.
};

/**
 * Worker function to load a portion of the graph data from a file.
 *
 * @param filename The name of the file to read from.
 * @param start_pos The starting position in the file.
 * @param end_pos The ending position in the file.
 * @param local_adjacency_list The local adjacency list to store the loaded data.
 */
void load_data_worker(const std::string &filename,
                      const std::streampos start_pos,
                      const std::streampos end_pos,
                      AdjacencyList &local_adjacency_list) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    file.seekg(start_pos); // Move to the starting position
    std::string line;
    while (file.tellg() < end_pos && std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            // Skip empty lines and comments
            continue;
        }

        int source, target; // Parse the source and target nodes
        std::istringstream iss(line); // Use string stream to parse the line
        iss >> source >> target;

        // Add the edge to the adjacency list
        local_adjacency_list.n_minus[source].push_back(target);
        local_adjacency_list.n_plus[target].push_back(source);
    }

    file.close();
}

/**
 * Function to load the entire graph data from a file using multiple threads.
 *
 * @param filename The name of the file to read from.
 * @return The adjacency list representing the graph.
 */
AdjacencyList load_data(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return {};
    }

    // Get the size of the file by moving the cursor to the end
    std::streampos file_size = file.tellg();
    file.seekg(0); // Move the cursor back to the beginning

    // Determine the number of threads to use and the chunk size for each thread
    const long num_threads = std::thread::hardware_concurrency();
    const long chunk_size = file_size / num_threads;
    std::vector<std::streampos> chunk_boundaries;
    chunk_boundaries.reserve(num_threads + 1);
    chunk_boundaries.emplace_back(0);

    // Calculate the boundaries for each chunk
    for (int i = 1; i < num_threads; ++i) {
        file.seekg(i * chunk_size);

        std::string dummy;
        std::getline(file, dummy);

        chunk_boundaries.push_back(file.tellg());
    }

    chunk_boundaries.push_back(file_size);
    file.close();

    // Create threads to load data in parallel
    std::vector<std::thread> threads;
    std::vector<AdjacencyList> partial_results(num_threads);

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(load_data_worker,
                             filename,
                             chunk_boundaries[i],
                             chunk_boundaries[i + 1],
                             std::ref(partial_results[i]));
    }

    for (auto &thread: threads) {
        thread.join();
    }

    // Combine the partial results into the final adjacency list
    AdjacencyList final_result;
    for (const auto &[n_minus, n_plus]: partial_results) {
        for (const auto &[key, values]: n_minus) {
            final_result.n_minus[key].insert(final_result.n_minus[key].end(), values.begin(), values.end());
        }
        for (const auto &[key, values]: n_plus) {
            final_result.n_plus[key].insert(final_result.n_plus[key].end(), values.begin(), values.end());
        }
    }

    return final_result;
}

/**
 * Function to get the total number of nodes in the graph.
 *
 * @param all_vertices The adjacency list representing the graph.
 * @return The total number of nodes.
 */
size_t get_total_node_count(const AdjacencyList &all_vertices) {
    std::unordered_map<int, std::vector<int> > all_vertices_map;

    for (const auto &[key, values]: all_vertices.n_minus) {
        all_vertices_map[key].insert(all_vertices_map[key].end(), values.begin(), values.end());
    }
    for (const auto &[key, values]: all_vertices.n_plus) {
        all_vertices_map[key].insert(all_vertices_map[key].end(), values.begin(), values.end());
    }

    return all_vertices_map.size();
}

/**
 * Worker function to compute a portion of the PageRank values.
 *
 * @param graph The adjacency list representing the graph.
 * @param old_pr The previous iteration's PageRank values.
 * @param new_pr The current iteration's PageRank values.
 * @param max_change The maximum change in PageRank values.
 * @param start The starting index for this worker.
 * @param end The ending index for this worker.
 * @param damping_factor The damping factor used in the PageRank calculation.
 */
void page_rank_worker(const AdjacencyList &graph,
                      const std::vector<double> &old_pr,
                      std::vector<double> &new_pr,
                      double &max_change,
                      const size_t start,
                      const size_t end,
                      const double damping_factor) {
    double local_max_change = 0.0;

    for (size_t i = start; i < end; i++) {
        double rank_sum = 0.0;

        // Calculate the sum of PageRank contributions from incoming neighbors
        if (auto it = graph.n_plus.find(static_cast<int>(i)); it != graph.n_plus.end()) {
            for (int neighbor: it->second) {
                if (auto neighbor_it = graph.n_minus.find(neighbor); neighbor_it != graph.n_minus.end()) {
                    rank_sum += old_pr[neighbor] / static_cast<double>(neighbor_it->second.size());
                }
            }
        }

        // Update the PageRank value using the formula:
        // PR(u) = (1 - d) / |V| + d * sum(PR(v) / |N+(v)|)
        new_pr[i] = (1.0 - damping_factor) / static_cast<double>(old_pr.size()) + damping_factor * rank_sum;
        local_max_change = std::max(local_max_change, std::fabs(new_pr[i] - old_pr[i]));
    }

    max_change = std::max(max_change, local_max_change);
}

/**
 * Function to compute the PageRank values for the graph.
 *
 * @param graph The adjacency list representing the graph.
 * @param total_nodes The total number of nodes in the graph.
 * @param damping_factor The damping factor used in the PageRank calculation.
 * @param threshold The convergence threshold.
 * @param max_iterations The maximum number of iterations.
 * @return The PageRank values for each node.
 */
std::vector<double> page_rank(const AdjacencyList &graph,
                              const size_t total_nodes,
                              double damping_factor = DAMPING_FACTOR,
                              const double threshold = EPSILON,
                              const int max_iterations = MAX_ITERATIONS) {
    // Initialize the PageRank values
    std::vector<double> page_rank(total_nodes, 1.0 / static_cast<double>(total_nodes));
    // Create a new vector to store the updated PageRank values
    std::vector<double> new_page_rank(total_nodes, 0.0);

    for (int iteration = 0; iteration < max_iterations; ++iteration) {
        std::cout << "Iteration " << iteration + 1 << std::endl;
        double max_change = 0.0; // Track the maximum change in PageRank values for convergence
        std::vector<std::thread> threads;
        const size_t chunk_size = total_nodes / max_iterations;
        for (size_t start = 0; start < total_nodes; start += chunk_size) {
            size_t end = std::min(start + chunk_size, total_nodes);
            threads.emplace_back(page_rank_worker,
                                 std::cref(graph),
                                 std::cref(page_rank),
                                 std::ref(new_page_rank),
                                 std::ref(max_change),
                                 start,
                                 end,
                                 damping_factor);
        }
        for (auto &thread: threads) {
            thread.join();
        }
        page_rank.swap(new_page_rank); // Swap the old and new PageRank values

        if (max_change < threshold) {
            std::cout << "Converged in " << iteration + 1 << " iterations." << std::endl;
            break;
        }
    }

    return page_rank;
}

/**
 * Function to print the top N nodes with the highest PageRank values.
 *
 * @param page_rank The PageRank values for each node.
 * @param top_n The number of top nodes to print.
 */
void print_top_n_nodes(const std::vector<double> &page_rank,
                       const size_t top_n = 10) {
    std::vector<std::pair<int, double> > node_ranks;
    // Pair each node with its PageRank value
    for (size_t i = 0; i < page_rank.size(); ++i) {
        node_ranks.emplace_back(static_cast<int>(i), page_rank[i]);
    }

    // Sort nodes by PageRank value in descending order
    std::sort(node_ranks.begin(), node_ranks.end(), [](const auto &a, const auto &b) {
        return b.second < a.second;
    });

    std::cout << "Top " << top_n << " nodes with highest PageRank:" << std::endl;

    // Print the top N nodes with the highest PageRank values
    for (size_t i = 0; i < top_n && i < node_ranks.size(); ++i) {
        std::cout << "Node " << node_ranks[i].first << ": " << node_ranks[i].second << std::endl;
    }
}


int main() {
    const std::string filename = "../project_3/web-BerkStan.txt";

    const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    const AdjacencyList all_vertices = load_data(filename);
    const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "Time for loading data: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
            << "ms" << std::endl;
    const size_t total_node_count = get_total_node_count(all_vertices);
    std::cout << "Total number of nodes: " << total_node_count << std::endl;

    const std::chrono::steady_clock::time_point begin_page_rank = std::chrono::steady_clock::now();
    std::vector<double> page_rank_values = page_rank(all_vertices, total_node_count);
    const std::chrono::steady_clock::time_point end_page_rank = std::chrono::steady_clock::now();
    std::cout << "Time for PageRank: " << std::chrono::duration_cast<std::chrono::milliseconds>(
        end_page_rank - begin_page_rank).count() << "ms" << std::endl;

    print_top_n_nodes(page_rank_values);

    return 0;
}
