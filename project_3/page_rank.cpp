#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
#include <string>

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
                      std::vector<std::string> &local_result) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    std::string line;
    size_t current_line = 0;

    while (current_line < start_line && std::getline(file, line)) {
        current_line++;
    }

    while (current_line < end_line && std::getline(file, line)) {
        line = trim_line(line);
        if (!line.empty()) {
            local_result.push_back(line);
        }
        current_line++;
    }

    file.close();
}

std::vector<std::string> load_data(const std::string &filename) {
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
    std::vector<std::vector<std::string> > partial_results(threads_count);

    for (size_t i = 0; i < threads_count; ++i) {
        size_t start_line = i * chunk_size;
        size_t end_line;
        if (i == threads_count - 1) {
            end_line = total_lines;
        } else {
            end_line = (i + 1) * chunk_size;
        }

        threads.emplace_back(load_data_worker, filename, start_line, end_line, std::ref(partial_results[i]));
    }

    for (auto &thread: threads) {
        thread.join();
    }

    std::vector<std::string> final_result;
    for (const auto &partial_result: partial_results) {
        final_result.insert(final_result.end(), partial_result.begin(), partial_result.end());
    }

    return final_result;
}

int main() {
    const std::string filename = "../project_3/web-BerkStan.txt";

    const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    const std::vector<std::string> data = load_data(filename);
    const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "Time for loading data: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
            << "ms" << std::endl;
    std::cout << "Loaded " << data.size() << " lines of data." << std::endl;

    return 0;
}
