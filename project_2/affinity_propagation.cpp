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


std::vector<std::vector<double> > calculate_similarity_matrix(const std::vector<std::vector<double> > &data) {
    const size_t size_n = data.size();
    const std::vector<double> row(size_n, 0);
    std::vector<std::vector<double> > similarity_matrix(size_n, row);

    double minimal_similarity = 0;
#pragma omp parallel for default(none) shared(data, similarity_matrix, size_n, minimal_similarity)
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
    print_matrix(similarity_matrix, "Similarity matrix");

    return similarity_matrix;
}

std::vector<std::vector<double> > calculate_affinity_propagation(const std::vector<std::vector<double> > &matrix_S,
                                                                 const double max_iteration) {
    const size_t size_n = matrix_S.size();
    const std::vector<double> row(size_n, 0);
    auto matrix_A = std::vector<std::vector<double> >(size_n, row);
    auto matrix_R = std::vector<std::vector<double> >(size_n, row);
    auto matrix_C = std::vector<std::vector<double> >(size_n, row);

    bool changed = true;
    double iteration = 0;

    while (changed && iteration < max_iteration) {
        iteration++;

        // Calculate responsibility matrix
#pragma omp parallel for collapse(2) default(none) shared(matrix_A, matrix_S, matrix_R, size_n)
        for (size_t i = 0; i < size_n; i++) {
            for (size_t k = 0; k < size_n; k++) {
                // S(i,k) part -> initialize R(i,k) with S(i,k)
                matrix_R[i][k] = matrix_S[i][k];

                // minus part
                double maximum = std::numeric_limits<double>::min();
                for (size_t k_ = 0; k_ < size_n; k_++) {
                    if (k_ != k) {
                        // max part of the equation -> max(A(i,k') + S(i,k'))
                        maximum = std::max(maximum, matrix_A[i][k_] + matrix_S[i][k_]);
                    }
                }
                // R(i,k) = S(i,k) - max part; we already did S(i,k) part, now we subtract max part
                matrix_R[i][k] -= maximum;
            }
        }

        // Calculate availability matrix
#pragma omp parallel for collapse(2) default(none) shared(matrix_A, matrix_R, size_n)
        for (size_t i = 0; i < size_n; i++) {
            for (size_t k = 0; k < size_n; k++) {
                double sum_on_max = 0;
                for (size_t i_ = 0; i_ < size_n; i_++) {
                    if (i_ != i) {
                        sum_on_max += std::max(0.0, matrix_R[i_][k]);
                    }
                }
                if (i == k) {
                    matrix_A[i][k] = sum_on_max;
                } else {
                    matrix_A[i][k] = std::min(0.0, sum_on_max + matrix_R[k][k]);
                }
            }
        }

        // Calculate C matrix
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
    }

    // print all matrices
    print_matrix(matrix_R, "R matrix");
    print_matrix(matrix_A, "A matrix");
    print_matrix(matrix_C, "C matrix");

    return matrix_C;
}

void create_clusters(const std::vector<std::vector<double> > &matrix_C) {
    auto max_values = std::vector<double>(matrix_C.size());
    auto max_indexes = std::vector<int64_t>(matrix_C.size());
    for (size_t i = 0; i < matrix_C.size(); i++) {
        auto a = std::ranges::max_element(matrix_C[i]);
        max_values[i] = *a;
        max_indexes[i] = std::distance(matrix_C[i].begin(), a);

        std::cout << "Row " << i + 1 << " belongs to cluster defined by element " << max_indexes[i] + 1 <<
                std::endl;
    }
    std::cout << std::endl;
}

int main() {
    int max_iteration = 1000;

    // five participants
    const std::string five_participant_file = "../project_2/five_participants.csv";
    const std::vector<std::string> five_participant_dataset = read_csv_file(five_participant_file);
    const std::vector<std::vector<double> > five_participant_matrix = tokenize_csv(five_participant_dataset);
    const std::vector<std::vector<double> > five_participants_similarity_matrix = calculate_similarity_matrix(
        five_participant_matrix);
    const std::vector<std::vector<double> > five_participants_clusters = calculate_affinity_propagation(
        five_participants_similarity_matrix, max_iteration);
    create_clusters(five_participants_clusters);


    // Iris dataset
    // const std::string iris_file = "../project_2/iris.csv";
    // const std::vector<std::string> iris_dataset = read_csv_file(iris_file);
    // const std::vector<std::vector<double> > iris_matrix = tokenize_csv(iris_dataset, ';');
    // const std::vector<std::vector<double> > iris_similarity_matrix = calculate_similarity_matrix(iris_matrix);
    // create_clusters(iris_similarity_matrix);


    // MNIST test dataset
    // const std::string mnist_file_test = "../project_2/mnist_test.csv";
    // const std::vector<std::string> mnist_test_dataset = read_csv_file(mnist_file_test);
    // const std::vector<std::vector<double> > mnist_test_matrix = tokenize_csv(mnist_test_dataset);
    // const std::vector<std::vector<double> > mnist_test_similarity_matrix = calculate_similarity_matrix(
    //     mnist_test_matrix);


    return 0;
}
