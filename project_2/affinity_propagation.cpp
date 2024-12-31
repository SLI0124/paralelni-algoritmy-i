#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <limits>

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

std::vector<std::vector<double> > tokenize_csv(const std::vector<std::string> &data, const char delimiter = ',') {
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

std::vector<std::vector<double> > calculate_similarity_matrix(const std::vector<std::vector<double> > &data,
                                                              const bool verbose) {
    const size_t size_n = data.size();
    const std::vector<double> row(size_n, 0);
    std::vector<std::vector<double> > similarity_matrix(size_n, row);

    double minimal_similarity = 0;
#pragma omp parallel for collapse(2) default(none) shared(data, similarity_matrix, size_n, minimal_similarity)
    for (size_t i = 0; i < size_n; i++) {
        for (size_t j = 0; j < size_n; j++) {
            double similarity = 0;

            for (size_t k = 0; k < data[i].size(); k++) {
                const double difference = data[i][k] - data[j][k];
                similarity += difference * difference;
            }
            similarity = -similarity;
            minimal_similarity = std::min(minimal_similarity, similarity);
            similarity_matrix[i][j] = similarity;
        }
    }

    // set diagonal to mean of minimal similarity
    for (size_t i = 0; i < size_n; i++) {
        similarity_matrix[i][i] = minimal_similarity;
    }

    // print similarity matrix
    if (verbose) {
        print_matrix(similarity_matrix, "Similarity matrix");
    }


    return similarity_matrix;
}

std::vector<std::vector<double> > calculate_affinity_propagation(const std::vector<std::vector<double> > &matrix_S,
                                                                 const int max_iteration,
                                                                 const bool verbose = false) {
    const size_t size_n = matrix_S.size();
    const std::vector<double> row(size_n, 0);
    auto matrix_A = std::vector<std::vector<double> >(size_n, row);
    auto matrix_R = std::vector<std::vector<double> >(size_n, row);
    auto matrix_C = std::vector<std::vector<double> >(size_n, row);

    bool changed = true;
    int iteration = 0;

    while (changed && iteration < max_iteration) {
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
#pragma omp parallel for collapse(2) default(none) shared(matrix_A, matrix_R, matrix_C, size_n, changed)
        for (size_t i = 0; i < size_n; i++) {
            for (size_t k = 0; k < size_n; k++) {
                const double new_value = matrix_A[i][k] + matrix_R[i][k];
                const double old_value = matrix_C[i][k];
                changed = changed || (new_value != old_value);
                matrix_C[i][k] = new_value;
            }
        }
        if (verbose) {
            print_matrix(matrix_C, "Combined Matrix after iteration " + std::to_string(iteration));
        }

        if (iteration == 1 && verbose) {
            // in assigment is this matrix used so we will satisfy the correct result
            create_clusters(matrix_C);
        }
    }

    // Print all matrices
    if (verbose) {
        print_matrix(matrix_R, "Final Responsibility Matrix");
        print_matrix(matrix_A, "Final Availability Matrix");
        print_matrix(matrix_C, "Final Combined Matrix");
    }

    std::cout << "Converged after " << iteration << " iterations" << std::endl;
    return matrix_C;
}

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
                max_value = matrix_C[i][j];
                max_index = static_cast<int>(j);
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
                // Cast to size_t
                std::cout << i << " ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}


int main() {
    constexpr int max_iteration = 1000;

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


    // Iris dataset
    // verbose = false;
    // const std::string iris_file = "../project_2/iris.csv";
    // const std::vector<std::string> iris_dataset = read_csv_file(iris_file);
    // const std::vector<std::vector<double> > iris_matrix = tokenize_csv(iris_dataset, ';');
    // const std::vector<std::vector<double> > iris_similarity_matrix = calculate_similarity_matrix(iris_matrix, verbose);
    // const std::vector<std::vector<double> > iris_clusters = calculate_affinity_propagation(iris_similarity_matrix,
    //     max_iteration, verbose);
    // create_clusters(iris_clusters);


    // MNIST test dataset
    // const std::string mnist_file_test = "../project_2/mnist_test.csv";
    // const std::vector<std::string> mnist_test_dataset = read_csv_file(mnist_file_test);
    // const std::vector<std::vector<double> > mnist_test_matrix = tokenize_csv(mnist_test_dataset);
    // const std::vector<std::vector<double> > mnist_test_similarity_matrix = calculate_similarity_matrix(
    //     mnist_test_matrix);


    return 0;
}
