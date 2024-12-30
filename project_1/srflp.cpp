#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <thread>

/**
 * Load the content of a file into a vector of strings.
 *
 * Opens the specified file and reads it line by line, storing each line
 * in a vector. If the file cannot be opened, an error is logged to stderr,
 * and an empty vector is returned.
 *
 * @param filename The name or path of the file to read.
 * @return A vector containing each line of the file as a string.
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
 * Compute the cost of a permutation of faculties based on the weight matrix.
 *
 * This function calculates the total weighted distance between pairs of faculties
 * arranged in a given permutation. The cost reflects the importance of placing
 * related faculties closer together.
 *
 * @param permutation A vector representing the permutation of faculties.
 * @param weights_matrix A square matrix where element [i][j] represents the weight between faculties i and j.
 * @param faculty_sizes A vector representing the size of each faculty.
 * @return The total cost as an unsigned long long to handle large values.
 */
unsigned long long calculate_cost(const std::vector<int> &permutation,
                                  const std::vector<std::vector<int> > &weights_matrix,
                                  const std::vector<int> &faculty_sizes) {
    unsigned long long cost = 0;

    // Calculate pairwise cost by iterating over the permutation
    for (size_t i = 0; i < permutation.size(); i++) {
        for (size_t j = i + 1; j < permutation.size(); j++) {
            // Calculate faculties in pair (i, j)
            const int faculty_1 = std::min(permutation[i], permutation[j]);
            const int faculty_2 = std::max(permutation[i], permutation[j]);
            // Get the weight between the two faculties from the matrix
            const unsigned long long weight = weights_matrix[faculty_1][faculty_2];

            // Calculate distance between faculties in the permutation
            int distance = (faculty_sizes[faculty_1] + faculty_sizes[faculty_2]) / 2;
            for (size_t k = i + 1; k < j; ++k) {
                distance += faculty_sizes[permutation[k]];
            }

            // Add weighted distance to the total cost
            cost += weight * distance;
        }
    }
    return cost;
}

/**
 * Evaluate a range of permutations to find the lowest-cost solution.
 *
 * This worker function is part of a parallelized Branch and Bound algorithm.
 * Each thread processes a subset of permutations and stores the best local result.
 *
 * @param faculties_sizes A vector of faculty sizes.
 * @param weights_matrix A square matrix representing weights between faculties.
 * @param permutations A list of permutations to evaluate.
 * @param local_best_cost A reference to store the best cost found by this worker.
 * @param local_best_permutation A reference to store the best permutation found by this worker.
 * @param start The index to start evaluating permutations from.
 * @param end The index to stop evaluating permutations.
 */
void branch_and_bound_worker(const std::vector<int> &faculties_sizes,
                             const std::vector<std::vector<int> > &weights_matrix,
                             const std::vector<std::vector<int> > &permutations,
                             unsigned long long &local_best_cost,
                             std::vector<int> &local_best_permutation,
                             const size_t start,
                             const size_t end) {
    // Initialize the best cost as maximum possible value
    local_best_cost = std::numeric_limits<unsigned long long>::max();

    // Evaluate each permutation in the assigned range
    for (size_t i = start; i < end; ++i) {
        // Calculate the cost of the current permutation
        const unsigned long long cost = calculate_cost(permutations[i], weights_matrix, faculties_sizes);
        // If the current cost is lower than the local best cost, update the local best cost and permutation
        if (cost < local_best_cost) {
            local_best_cost = cost;
            local_best_permutation = permutations[i];
        }
    }
}

/**
 * Perform Branch and Bound to solve the Single Row Facility Layout Problem (SRFLP).
 *
 * This function generates all permutations of faculties and uses multi-threading
 * to divide the workload. It searches for the permutation with the lowest cost.
 *
 * @param faculties_sizes A vector representing the size of each faculty.
 * @param weights_matrix A square matrix representing weights between faculties.
 */
void branch_and_bound(const std::vector<int> &faculties_sizes,
                      const std::vector<std::vector<int> > &weights_matrix) {
    // Initialize the base permutation (0 to n-1)
    std::vector<int> base_permutation(faculties_sizes.size());
    for (std::vector<int>::size_type i = 0; i < base_permutation.size(); ++i) {
        base_permutation[i] = static_cast<int>(i);
    }

    // Generate all permutations
    std::vector<std::vector<int> > all_permutations;
    all_permutations.push_back(base_permutation);
    while (std::ranges::next_permutation(base_permutation).found) {
        all_permutations.push_back(base_permutation);
    }

    // Initialize the global best cost
    unsigned long long best_cost = std::numeric_limits<unsigned long long>::max();
    std::vector<int> best_permutation;

    // Determine the number of threads to use based on the hardware concurrency
    const size_t number_of_threads = std::thread::hardware_concurrency();
    std::cout << "Number of threads: " << number_of_threads << std::endl;
    const size_t total_permutations = all_permutations.size();
    std::cout << "Total permutations: " << total_permutations << std::endl;
    const size_t chunk_size = total_permutations / number_of_threads;
    std::cout << "Chunk size: " << chunk_size << std::endl;

    // Create vectors to store threads, local costs, and local permutations
    std::vector<std::thread> threads;
    std::vector<unsigned long long> local_costs(number_of_threads);
    std::vector<std::vector<int> > local_permutations(number_of_threads);

    // Launch threads to evaluate subsets of permutations
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
    }

    // Wait for all threads to complete
    for (auto &thread: threads) {
        thread.join();
    }

    // Find the best result across all threads
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
