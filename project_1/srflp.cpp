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


void branch_and_bound() {
    // TODO: Finally understand what the hell is required here and what to do and parallelize it
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

    std::cout << "Number of rows and columns: " << number_of_rows << std::endl;
    std::cout << "Sizes: ";
    for (const auto &s: faculties_sizes) {
        std::cout << s << " ";
    }
    std::cout << std::endl;

    std::cout << "Matrix:" << std::endl;
    for (const auto &row: weights_matrix) {
        for (const auto &elem: row) {
            std::cout << elem << " ";
        }
        std::cout << std::endl;
    }

    unsigned int worker_count = std::thread::hardware_concurrency();
    std::cout << "Number of threads: " << worker_count << std::endl;

    unsigned int total_permutations = 1;
    for (unsigned int i = 1; i <= number_of_rows; ++i) {
        total_permutations *= i;
    }
    std::cout << "Total permutations: " << total_permutations << std::endl;
    unsigned int chunk_size = total_permutations / worker_count;
    std::cout << "Chunk size: " << chunk_size << std::endl;

    // Test the cost calculation
    {
        std::vector<int> test_permutation = {6, 5, 4, 3, 2, 1, 0, 7, 8, 9};
        unsigned long long test_cost = calculate_cost(test_permutation, weights_matrix, faculties_sizes);
        std::cout << "Test cost: " << test_cost << std::endl;

        std::vector<int> allegedly_best_permutation = {0, 4, 1, 9, 6, 3, 7, 2, 5, 8};
        unsigned long long allegedly_best_cost =
                calculate_cost(allegedly_best_permutation, weights_matrix, faculties_sizes);
        std::cout << "Allegedly best cost: " << allegedly_best_cost << std::endl;

        // revert the order of the faculties - they should gave the same cost
        std::vector<int> allegedly_best_permutation_reversed = allegedly_best_permutation;
        std::reverse(allegedly_best_permutation_reversed.begin(), allegedly_best_permutation_reversed.end());
        unsigned long long allegedly_best_cost_reversed =
                calculate_cost(allegedly_best_permutation_reversed, weights_matrix, faculties_sizes);
        std::cout << "Allegedly best cost reversed: " << allegedly_best_cost_reversed << std::endl;
    }

    return 0;
}
