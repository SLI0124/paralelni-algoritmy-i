#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <thread>


/**
 * Load a file and return its content as a vector of strings.
 *
 * This function opens a file with the given filename, reads its content line by line,
 * and stores each line in a vector of strings. If the file cannot be opened, an error
 * message is printed to the standard error stream, and an empty vector is returned.
 *
 * @param filename The name of the file to load.
 * @return A vector of strings containing the lines of the file.
 */
std::vector<std::string> load_file(const std::string &filename) {
    std::vector<std::string> data;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open the file " << filename << std::endl;
        return data;
    }

    std::string line;
    while (std::getline(file, line)) {
        data.push_back(line);
    }

    file.close();
    return data;
}


/**
 * Calculate the cost of a permutation between the faculties based on the given matrix and sizes.
 *
 * This function computes the cost of a given permutation of faculties. The cost is calculated
 * based on the weights between pairs of faculties and the distances between them. The distance
 * is determined by the sizes of the faculties and the order in the permutation.
 *
 * @param permutation The permutation of the faculties.
 * @param weights_matrix The matrix containing the weights of the edges between the faculties.
 * @param faculty_sizes The sizes of the faculties.
 * @return The cost of the permutation as an unsigned long long to avoid overflow.
 */
unsigned long long calculate_cost(const std::vector<int> &permutation,
                                  const std::vector<std::vector<int> > &weights_matrix,
                                  const std::vector<int> &faculty_sizes) {
    unsigned long long cost = 0;

    // Iterate over each pair of faculties in the permutation
    for (size_t i = 0; i < permutation.size(); i++) {
        for (size_t j = i + 1; j < permutation.size(); j++) {
            // Determine the two faculties in the pair, get the upper triangle of the matrix since we have data there
            const int faculty_1 = std::min(permutation[i], permutation[j]);
            const int faculty_2 = std::max(permutation[i], permutation[j]);
            // Get the weight between the two faculties
            const unsigned long long weight = weights_matrix[faculty_1][faculty_2];

            // Calculate the distance between the two faculties
            int distance = (faculty_sizes[faculty_1] + faculty_sizes[faculty_2]) / 2;
            for (size_t k = i + 1; k < j; ++k) {
                distance += faculty_sizes[permutation[k]];
            }
            // Add the weighted distance to the total cost
            cost += weight * distance;
        }
    }
    return cost;
}


void branch_and_bound_worker(const std::vector<int> &faculties_sizes,
                             const std::vector<std::vector<int> > &weights_matrix,
                             const std::vector<std::vector<int> > &permutations,
                             unsigned long long &local_best_cost,
                             std::vector<int> &local_best_permutation,
                             const size_t start,
                             const size_t end) {
    local_best_cost = std::numeric_limits<unsigned long long>::max();

    for (size_t i = start; i < end; ++i) {
        const unsigned long long cost = calculate_cost(permutations[i], weights_matrix, faculties_sizes);
        if (cost < local_best_cost) {
            local_best_cost = cost;
            local_best_permutation = permutations[i];
        }
    }
}


void branch_and_bound(const std::vector<int> &faculties_sizes,
                      const std::vector<std::vector<int> > &weights_matrix) {
    std::vector<int> base_permutation(faculties_sizes.size());
    for (int i = 0; i < base_permutation.size(); ++i) {
        base_permutation[i] = i;
    }

    std::vector<std::vector<int> > all_permutations;
    all_permutations.push_back(base_permutation);
    while (std::ranges::next_permutation(base_permutation).found) {
        all_permutations.push_back(base_permutation);
    }

    unsigned long long best_cost = std::numeric_limits<unsigned long long>::max();
    std::vector<int> best_permutation;

    const size_t number_of_threads = std::thread::hardware_concurrency();
    std::cout << "Number of threads: " << number_of_threads << std::endl;
    const size_t total_permutations = all_permutations.size();
    std::cout << "Total permutations: " << total_permutations << std::endl;
    const size_t chunk_size = total_permutations / number_of_threads;
    std::cout << "Chunk size: " << chunk_size << std::endl;

    std::vector<std::thread> threads;
    std::vector<unsigned long long> local_costs(number_of_threads);
    std::vector<std::vector<int> > local_permutations(number_of_threads);

    for (size_t i = 0; i < number_of_threads; i++) {
        const size_t start = i * chunk_size;
        size_t end;
        if (i == number_of_threads - 1) {
            end = total_permutations;
        } else {
            end = (i + 1) * chunk_size;
        }

        threads.emplace_back(branch_and_bound_worker,
                             std::cref(faculties_sizes),
                             std::cref(weights_matrix),
                             std::cref(all_permutations),
                             std::ref(local_costs[i]),
                             std::ref(local_permutations[i]),
                             start,
                             end);
    };

    for (auto &thread: threads) {
        thread.join();
    }

    for (size_t i = 0; i < number_of_threads; ++i) {
        if (local_costs[i] < best_cost) {
            best_cost = local_costs[i];
            best_permutation = local_permutations[i];
        }
    }

    std::cout << "Best cost: " << best_cost << std::endl;
    std::cout << "Best permutation: ";
    for (const int faculty: best_permutation) {
        std::cout << faculty << " ";
    }
}

int main() {
    const std::string filename = "../project_1/Y-10_t.txt";
    std::vector<std::string> data = load_file(filename);

    if (data.empty()) {
        return 1;
    }

    int number_of_rows = std::stoi(data[0]);

    std::vector<int> faculties_sizes;
    std::istringstream sizes_stream(data[1]);
    int size;
    while (sizes_stream >> size) {
        faculties_sizes.push_back(size);
    }

    std::vector<std::vector<int> > weights_matrix(number_of_rows, std::vector<int>(number_of_rows, 0));
    for (int i = 0; i < number_of_rows; ++i) {
        std::istringstream row_stream(data[i + 2]);
        for (int j = 0; j < number_of_rows; ++j) {
            row_stream >> weights_matrix[i][j];
        }
    }

    branch_and_bound(faculties_sizes, weights_matrix);

    return 0;
}
