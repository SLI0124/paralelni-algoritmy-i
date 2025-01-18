#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <unordered_map>
#include <sstream>
#include <chrono>

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


int main() {
    const std::string filename = "../project_3/web-BerkStan.txt";

    const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    const AdjacencyList all_vertices = load_data(filename);
    const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "Time for loading data: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
            << "ms" << std::endl;
    const size_t total_node_count = get_total_node_count(all_vertices);
    std::cout << "Total number of nodes: " << total_node_count << std::endl;

    return 0;
}
