#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <limits>

/**
 * Reads a CSV file and returns its content as a vector of strings.
 * Each string represents a line in the CSV file.
 *
 * @param filename The name of the CSV file to read.
 * @return A vector of strings containing the lines of the CSV file.
 */
std::vector<std::string> read_csv_file(const std::string &filename) {
    std::vector<std::string> data;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open the file " << filename << std::endl;
        return data;
    }

    std::string line;

    // skip the header
    std::getline(file, line);

    while (std::getline(file, line)) {
        data.push_back(line);
    }
    file.close();

    return data;
}

/**
 * Tokenizes the lines of a CSV file into a 2D vector of doubles.
 *
 * @param data A vector of strings where each string is a line from the CSV file.
 * @param delimiter The character used to separate values in the CSV file.
 * @return A 2D vector of doubles representing the tokenized CSV data.
 */
std::vector<std::vector<double> > tokenize_csv(const std::vector<std::string> &data,
                                               const char delimiter = ',') {
    std::vector<std::vector<double> > result;
    for (const std::string &line: data) {
        std::vector<double> row;
        std::string token;
        std::istringstream tokenStream(line);
        while (std::getline(tokenStream, token, delimiter)) {
            row.push_back(std::stod(token));
        }
        result.push_back(row);
    }
    return result;
}

/**
 * Prints a 2D matrix to the console.
 *
 * @param matrix The 2D vector of doubles to print.
 * @param name The name of the matrix to print.
 */
void print_matrix(const std::vector<std::vector<double> > &matrix, const std::string &name) {
    std::cout << name << std::endl;

    if (matrix.empty()) {
        std::cout << "Empty matrix" << std::endl;
        return;
    }

    for (const auto &row: matrix) {
        for (const auto &element: row) {
            std::cout << element << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

// forward declaration for better code readability
void create_clusters(const std::vector<std::vector<double> > &matrix_C);

/**
 * Calculates the similarity matrix for the given data.
 *
 * @param data A 2D vector of doubles representing the input data.
 * @param verbose A boolean flag to indicate whether to print the similarity matrix.
 * @return A 2D vector of doubles representing the similarity matrix.
 */
std::vector<std::vector<double> > calculate_similarity_matrix(const std::vector<std::vector<double> > &data,
                                                              const bool verbose) {
    const size_t size_n = data.size(); // dimension of the data, meaning first dimension of the matrix
    const std::vector<double> row(size_n, 0); // initialize a row with zeros, meaning second dimension of the matrix
    std::vector<std::vector<double> > similarity_matrix(size_n, row); // initialize a matrix with zeros

    // initialize the minimal similarity to 0, it helps us to set the diagonal of the similarity matrix
    double minimal_similarity = 0;
#pragma omp parallel for collapse(2) default(none) shared(data, similarity_matrix, size_n) reduction(min: minimal_similarity)
    for (size_t i = 0; i < size_n; i++) {
        for (size_t j = 0; j < size_n; j++) {
            double similarity = 0;
            // calculate the similarity between data[i] and data[j]
            for (size_t k = 0; k < data[i].size(); k++) {
                const double difference = data[i][k] - data[j][k];
                similarity += difference * difference;
            }
            // formula part for the similarity
            similarity = -similarity;
            minimal_similarity = std::min(minimal_similarity, similarity);
            similarity_matrix[i][j] = similarity;
        }
    }
    // set diagonal to mean of minimal similarity
    for (size_t i = 0; i < size_n; i++) {
        similarity_matrix[i][i] = minimal_similarity;
    }

    if (verbose) {
        print_matrix(similarity_matrix, "Similarity matrix");
    }
    std::cout << "Similarity matrix calculated" << std::endl;

    return similarity_matrix;
}

/**
 * Performs affinity propagation clustering on the given similarity matrix.
 *
 * @param matrix_S A 2D vector of doubles representing the similarity matrix.
 * @param max_iteration The maximum number of iterations to perform.
 * @param verbose A boolean flag to indicate whether to print intermediate matrices.
 * @return A 2D vector of doubles representing the final combined matrix.
 */
std::vector<std::vector<double> > calculate_affinity_propagation(const std::vector<std::vector<double> > &matrix_S,
                                                                 const int max_iteration,
                                                                 const bool verbose = false) {
    const size_t size_n = matrix_S.size(); // same as before, first dimension of the matrix
    const std::vector<double> row(size_n, 0); // same as before, second dimension of the matrix
    auto matrix_A = std::vector<std::vector<double> >(size_n, row);
    auto matrix_R = std::vector<std::vector<double> >(size_n, row);
    auto matrix_C = std::vector<std::vector<double> >(size_n, row);

    // here we will keep track of variables that are responsible for last change and stopping condition
    bool changed = true; // flag to indicate if the matrix has changed or not
    int iteration = 0; // iteration counter

    while (changed && iteration < max_iteration) {
        std::cout << "Iteration " << iteration << " out of " << max_iteration << std::endl;
        iteration++;

        // Calculate responsibility matrix
#pragma omp parallel for collapse(2) default(none) shared(matrix_A, matrix_S, matrix_R, size_n)
        for (size_t i = 0; i < size_n; i++) {
            for (size_t k = 0; k < size_n; k++) {
                double max_val = -std::numeric_limits<double>::infinity();

                // Find max over all k' != k
                for (size_t k_ = 0; k_ < size_n; k_++) {
                    if (k_ != k) {
                        double value = matrix_A[i][k_] + matrix_S[i][k_];
                        max_val = std::max(max_val, value);
                    }
                }

                // Update the responsibility matrix
                matrix_R[i][k] = matrix_S[i][k] - max_val;
            }
        }
        if (verbose) {
            print_matrix(matrix_R, "Responsibility Matrix after iteration " + std::to_string(iteration));
        }

        // Calculate availability matrix
#pragma omp parallel for collapse(2) default(none) shared(matrix_A, matrix_R, size_n)
        for (size_t i = 0; i < size_n; i++) {
            for (size_t k = 0; k < size_n; k++) {
                if (i != k) {
                    double sum = 0;

                    // Sum over all i' != i, i' != k
                    for (size_t i_ = 0; i_ < size_n; i_++) {
                        if (i_ != i && i_ != k) {
                            sum += std::max(0.0, matrix_R[i_][k]);
                        }
                    }

                    // Off-diagonal elements
                    matrix_A[i][k] = std::min(0.0, matrix_R[k][k] + sum);
                }

                // Diagonal elements (i == k)
                if (i == k) {
                    double sum = 0;
                    for (size_t i_ = 0; i_ < size_n; i_++) {
                        if (i_ != k) {
                            sum += std::max(0.0, matrix_R[i_][k]);
                        }
                    }
                    matrix_A[k][k] = sum;
                }
            }
        }
        if (verbose) {
            print_matrix(matrix_A, "Availability Matrix after iteration " + std::to_string(iteration));
        }

        // Calculate combined matrix C
        changed = false;
#pragma omp parallel for collapse(2) default(none) shared(matrix_A, matrix_R, matrix_C, size_n) reduction(||: changed)
        for (size_t i = 0; i < size_n; i++) {
            for (size_t k = 0; k < size_n; k++) {
                const double new_value = matrix_A[i][k] + matrix_R[i][k];
                const double old_value = matrix_C[i][k];
                // Check if the value has changed to start next iteration
                if (new_value != old_value) {
                    changed = true;
                }
                matrix_C[i][k] = new_value;
            }
        }
        if (verbose) {
            print_matrix(matrix_C, "Combined Matrix after iteration " + std::to_string(iteration));
        }

        if (iteration == 1 && verbose) {
            // in assignment is this matrix used so we will satisfy the correct result
            create_clusters(matrix_C);
        }
    }

    // Print all matrices
    if (verbose) {
        print_matrix(matrix_R, "Final Responsibility Matrix");
        print_matrix(matrix_A, "Final Availability Matrix");
        print_matrix(matrix_C, "Final Combined Matrix");
    }

    std::cout << "Converged after " << iteration << " iterations" << std::endl << std::endl;
    return matrix_C;
}

/**
 * Creates clusters based on the combined matrix C.
 *
 * @param matrix_C A 2D vector of doubles representing the combined matrix.
 */
void create_clusters(const std::vector<std::vector<double> > &matrix_C) {
    const size_t num_rows = matrix_C.size();
    const size_t num_cols = matrix_C[0].size();

    // Create a vector to store the cluster assignments
    std::vector<int> cluster_assignments(num_rows, -1);

    // For each row, find the column with the maximum value and assign the cluster
    for (size_t i = 0; i < num_rows; i++) {
        double max_value = std::numeric_limits<double>::lowest();
        int max_index = -1;

        for (size_t j = 0; j < num_cols; j++) {
            if (matrix_C[i][j] > max_value) {
                // find the maximum value in the row
                max_value = matrix_C[i][j]; // update the maximum value
                max_index = static_cast<int>(j); // update the index of the maximum value
            }
        }

        // Assign the cluster (class) based on the column index with the highest value
        cluster_assignments[i] = max_index;
    }

    // Print the cluster assignments
    std::cout << "Cluster assignments:" << std::endl;
    // iterate over each cluster and print the indices of the data points in that cluster
    for (size_t cluster = 0; cluster < num_cols; cluster++) {
        std::cout << "Cluster " << cluster << ": ";
        for (size_t i = 0; i < num_rows; i++) {
            if (static_cast<size_t>(cluster_assignments[i]) == cluster) {
                // Cast to size_t for .clang-tidy
                std::cout << i << " ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}


int main() {
    constexpr int max_iteration = 100;

    // five participants
    bool verbose = true;
    const std::string five_participant_file = "../project_2/five_participants.csv";
    const std::vector<std::string> five_participant_dataset = read_csv_file(five_participant_file);
    const std::vector<std::vector<double> > five_participant_matrix = tokenize_csv(five_participant_dataset);
    const std::vector<std::vector<double> > five_participants_similarity_matrix = calculate_similarity_matrix(
        five_participant_matrix, verbose);
    const std::vector<std::vector<double> > five_participants_clusters = calculate_affinity_propagation(
        five_participants_similarity_matrix, max_iteration, verbose);
    create_clusters(five_participants_clusters);

    // // MNIST test dataset, it's way too big so it takes ages (it did not finish on my machine), uncomment on your own risk
    // verbose = false;
    // const std::string mnist_file_test = "../project_2/mnist_test.csv";
    // const std::vector<std::string> mnist_test_dataset = read_csv_file(mnist_file_test);
    // const std::vector<std::vector<double> > mnist_test_matrix = tokenize_csv(mnist_test_dataset);
    // const std::vector<std::vector<double> > mnist_test_similarity_matrix = calculate_similarity_matrix(
    //     mnist_test_matrix, verbose);
    // const std::vector<std::vector<double> > mnist_test_clusters = calculate_affinity_propagation(
    //     mnist_test_similarity_matrix, max_iteration, verbose);
    // create_clusters(mnist_test_clusters);

    return 0;
}
