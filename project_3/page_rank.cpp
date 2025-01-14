#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>

void load_data_worker(const std::string &file_name,
                      const unsigned int start,
                      const unsigned int end,
                      std::vector<std::string> &result) {
    std::ifstream file(file_name);
    if (!file.is_open()) {
        std::cerr << "Error: file not found" << std::endl;
        return;
    }

    file.seekg(start);
    std::string line;
    while (file.tellg() < end && std::getline(file, line)) {
        result.push_back(line);
    }
}

std::vector<std::string> load_data(const std::string &file_name) {
    std::ifstream file(file_name);
    if (!file.is_open()) {
        std::cerr << "Error: file not found" << std::endl;
        return {};
    }

    file.seekg(0, std::ios::end);
    long file_size = file.tellg();

    unsigned int number_of_threads = std::max(1u, std::thread::hardware_concurrency());
    unsigned int chunk_size = file_size / number_of_threads;

    std::vector<std::thread> threads;
    std::vector<std::vector<std::string> > results(number_of_threads);

    for (unsigned int i = 0; i < number_of_threads; i++) {
        unsigned int start = i * chunk_size;
        unsigned int end = (i == number_of_threads - 1) ? file_size : (i + 1) * chunk_size;

        threads.emplace_back(load_data_worker, file_name, start, end, std::ref(results[i]));
    }

    for (std::thread &t: threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    std::vector<std::string> final_result;
    for (const auto &part: results) {
        final_result.insert(final_result.end(), part.begin(), part.end());
    }

    return final_result;
}

int main() {
    const std::string file_path = "../project_3/web-BerkStan.txt";

    const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    const std::vector<std::string> data = load_data(file_path);
    const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time for loading data: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
            << "ms" << std::endl;

    std::cout << "Loaded " << data.size() << " lines of data." << std::endl;

    return 0;
}
