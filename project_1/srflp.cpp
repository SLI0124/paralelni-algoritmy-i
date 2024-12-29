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


/**
 * Calculate the cost of a permutation between the faculties based on the given matrix and sizes.
 *
 * This function computes the cost of a given permutation of faculties. The cost is calculated
 * based on the weights between pairs of faculties and the distances between them. The distance
 * is determined by the sizes of the faculties and the order in the permutation.
 *
 * @param faculties_sizes The sizes of the faculties.
 * @param weights_matrix The matrix containing the weights of the edges between the faculties.
 */
void parallel_brute_force(const std::vector<int> &faculties_sizes,
                          const std::vector<std::vector<int> > &weights_matrix) {
    // Initialize the base permutation with indices 0 to faculties_sizes.size() - 1
    std::vector<int> base_permutation(faculties_sizes.size());
    for (int i = 0; i < faculties_sizes.size(); i++) {
        base_permutation[i] = i;
    }

    // Generate all permutations of the base permutation, nice little function from the ranges library shown in class
    std::vector<std::vector<int> > all_permutations;
    all_permutations.push_back(base_permutation); // don't forget to add the base permutation
    while (std::ranges::next_permutation(base_permutation).found) {
        all_permutations.push_back(base_permutation);
    }

    const size_t total_permutations = all_permutations.size();
    std::cout << "Total permutations: " << total_permutations << std::endl;
    const size_t chunk_size = total_permutations / std::thread::hardware_concurrency();
    std::cout << "Chunk size: " << chunk_size << std::endl;

    // Initialize the best cost to the maximum value
    unsigned long long best_cost = std::numeric_limits<unsigned long long>::max();
    std::vector<int> best_permutation; // Initialize the best permutation to an empty vector

    // Create threads to calculate the cost of permutations in parallel
    std::vector<std::thread> threads;
    for (size_t i = 0; i < std::thread::hardware_concurrency(); i++) {
        threads.emplace_back(
            /*
             * i: The index of the thread. Used to determine the start and end of the chunk.
             * chunk_size: The size of the chunk assigned to the thread.
             * all_permutations: The vector containing all permutations.
             * faculties_sizes: The sizes of the faculties.
             * weights_matrix: The matrix containing the weights of the edges between the faculties.
             * best_cost: The best cost found so far. Updated if a better cost is found.
             * best_permutation: The best permutation found so far. Updated if a better permutation is found.
             */
            [i, chunk_size, &all_permutations, &faculties_sizes, &weights_matrix, &best_cost, &best_permutation]() {
                const size_t start = i * chunk_size; // Calculate the start of the assigned chunk
                size_t end = (i + 1) * chunk_size; // Calculate the end of the assigned chunk
                // Adjust the end for the last thread
                if (i == std::thread::hardware_concurrency() - 1) {
                    end = all_permutations.size(); // The last thread gets the remaining permutations
                }

                // Calculate the cost for each permutation in the assigned chunk
                for (size_t j = start; j < end; ++j) {
                    const unsigned long long cost = // Calculate the cost of the permutation
                            calculate_cost(all_permutations[j], weights_matrix, faculties_sizes);
                    // Update the best cost and permutation if a better cost is found
                    if (cost < best_cost) {
                        best_cost = cost; // Update the best cost
                        best_permutation = all_permutations[j]; // Update the best permutation
                    }
                }
            });
    }

    std::cout << std::endl << "Threads created" << std::endl << std::endl;
    for (auto &thread: threads) {
        // Wait for all threads to finish
        thread.join();
    }

    std::cout << "Best cost: " << best_cost << std::endl;
    std::cout << "Best permutation: ";
    for (const auto &elem: best_permutation) {
        std::cout << elem << " ";
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

    parallel_brute_force(faculties_sizes, weights_matrix);

    return 0;
}
