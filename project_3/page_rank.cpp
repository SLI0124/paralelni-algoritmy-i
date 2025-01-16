#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <unordered_map>
#include <sstream>
#include <chrono>

struct Vertex {
    std::unordered_map<int, std::vector<int> > n_minus;
    std::unordered_map<int, std::vector<int> > n_plus;
};

std::string trim_line(const std::string &str) {
    size_t start = 0;
    while (start < str.size() && isspace(str[start])) {
        start++;
    }

    size_t end = str.size();
    while (end > start && isspace(str[end - 1])) {
        end--;
    }

    return str.substr(start, end - start);
}

void load_data_worker(const std::string &filename,
                      const size_t start_line,
                      const size_t end_line,
                      std::vector<Vertex> &local_vertices) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    std::string line;
    size_t current_line = start_line;

    while (current_line < end_line && std::getline(file, line)) {
        line = trim_line(line);
        if (line.empty() || line[0] == '#') {
            current_line++;
            continue;
        }

        std::istringstream iss(line);
        int from, to;
        iss >> from >> to;

        Vertex vertex;
        vertex.n_minus[from].push_back(to);
        vertex.n_plus[to].push_back(from);
        local_vertices.push_back(vertex);

        current_line++;
    }

    file.close();
}

std::vector<Vertex> load_data(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return {};
    }

    size_t total_lines = 0;
    std::string line;
    while (std::getline(file, line)) {
        total_lines++;
    }
    file.close();

    const size_t threads_count = std::thread::hardware_concurrency();
    const size_t chunk_size = total_lines / threads_count;
    std::vector<std::thread> threads;
    std::vector<std::vector<Vertex> > partial_results(threads_count);

    for (size_t i = 0; i < threads_count; ++i) {
        size_t start_line = i * chunk_size;
        size_t end_line = (i == threads_count - 1) ? total_lines : (i + 1) * chunk_size;

        threads.emplace_back(load_data_worker, filename, start_line, end_line, std::ref(partial_results[i]));
    }

    for (auto &thread: threads) {
        thread.join();
    }

    std::vector<Vertex> final_result;
    for (const auto &partial_result: partial_results) {
        final_result.insert(final_result.end(), partial_result.begin(), partial_result.end());
    }

    return final_result;
}

int main() {
    const std::string filename = "../project_3/web-BerkStan.txt";

    const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    const std::vector<Vertex> all_vertices = load_data(filename);
    const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "Time for loading data: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
            << "ms" << std::endl;
    std::cout << "Loaded " << all_vertices.size() << " vertices." << std::endl;

    return 0;
}
