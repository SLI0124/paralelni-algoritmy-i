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

struct AdjacencyList {
    std::unordered_map<int, std::vector<int> > n_minus;
    std::unordered_map<int, std::vector<int> > n_plus;
};

void load_data_worker(const std::string &filename,
                      const std::streampos start_pos,
                      const std::streampos end_pos,
                      AdjacencyList &local_adjacency_list) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    file.seekg(start_pos);
    std::string line;
    while (file.tellg() < end_pos && std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        int source, target;
        std::istringstream iss(line);
        iss >> source >> target;

        local_adjacency_list.n_minus[source].push_back(target);
        local_adjacency_list.n_plus[target].push_back(source);
    }

    file.close();
}

AdjacencyList load_data(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return {};
    }

    std::streampos file_size = file.tellg();
    file.seekg(0);

    const long num_threads = std::thread::hardware_concurrency();
    const long chunk_size = file_size / num_threads;
    std::vector<std::streampos> chunk_boundaries;
    chunk_boundaries.reserve(num_threads + 1);
    chunk_boundaries.emplace_back(0);

    for (int i = 1; i < num_threads; ++i) {
        file.seekg(i * chunk_size);

        std::string dummy;
        std::getline(file, dummy);

        chunk_boundaries.push_back(file.tellg());
    }

    chunk_boundaries.push_back(file_size);
    file.close();

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

void page_rank_worker(const AdjacencyList &graph,
                      const std::vector<double> &old_pr,
                      std::vector<double> &new_pr,
                      double &max_change,
                      const size_t start,
                      const size_t end,
                      const double damping_factor) {
    double local_max_change = 0.0;

    for (size_t i = start; i < end; ++i) {
        double rank_sum = 0.0;

        if (auto it = graph.n_plus.find(static_cast<int>(i)); it != graph.n_plus.end()) {
            for (int neighbor: it->second) {
                if (auto neighbor_it = graph.n_minus.find(neighbor); neighbor_it != graph.n_minus.end()) {
                    rank_sum += old_pr[neighbor] / static_cast<double>(neighbor_it->second.size());
                }
            }
        }
        new_pr[i] = (1.0 - damping_factor) / static_cast<double>(old_pr.size()) + damping_factor * rank_sum;
        local_max_change = std::max(local_max_change, std::fabs(new_pr[i] - old_pr[i]));
    }

    max_change = std::max(max_change, local_max_change);
}

std::vector<double> page_rank(const AdjacencyList &graph,
                              const size_t total_nodes,
                              double damping_factor = DAMPING_FACTOR,
                              const double threshold = EPSILON,
                              const int max_iterations = MAX_ITERATIONS) {
    std::vector<double> page_rank(total_nodes, 1.0 / static_cast<double>(total_nodes));
    std::vector<double> new_page_rank(total_nodes, 0.0);

    for (int iteration = 0; iteration < max_iterations; ++iteration) {
        std::cout << "Iteration " << iteration + 1 << std::endl;
        double max_change = 0.0;
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
        page_rank.swap(new_page_rank);

        if (max_change < threshold) {
            std::cout << "Converged in " << iteration + 1 << " iterations." << std::endl;
            break;
        }
    }

    return page_rank;
}


void print_top_n_nodes(const std::vector<double> &page_rank,
                       const size_t top_n = 10) {
    std::vector<std::pair<int, double> > node_ranks;
    for (size_t i = 0; i < page_rank.size(); ++i) {
        node_ranks.emplace_back(static_cast<int>(i), page_rank[i]);
    }

    std::sort(node_ranks.begin(), node_ranks.end(), [](const auto &a, const auto &b) {
        return b.second < a.second;
    });

    std::cout << "Top " << top_n << " nodes with highest PageRank:" << std::endl;
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
