#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

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

std::vector<std::vector<double> > tokenize_csv(const std::vector<std::string> &data) {
    std::vector<std::vector<double> > result;
    for (const std::string &line: data) {
        std::vector<double> row;
        std::string token;
        std::istringstream tokenStream(line);
        while (std::getline(tokenStream, token, ',')) {
            row.push_back(std::stod(token));
        }
        result.push_back(row);
    }
    return result;
}

std::vector<std::vector<double> > calculate_similarity_matrix(const std::vector<std::vector<double> > &data) {
    const size_t size_n = data.size();
    const std::vector<double> row(size_n, 0);
    std::vector<std::vector<double> > similarity_matrix(size_n, row);

    double minimal_similarity = 0;
#pragma omp parallel for default(none) shared(data, similarity_matrix, size_n, minimal_similarity)
    for (int i = 0; i < size_n; i++) {
        for (int j = 0; j < size_n; j++) {
            double similarity = 0;

            for (int k = 0; k < data[i].size(); k++) {
                similarity += (data[i][k] == data[j][k]);
            }
            similarity = -similarity;
            minimal_similarity = std::min(minimal_similarity, similarity);
            similarity_matrix[i][j] = similarity;
        }
    }

    // not sure what to set the diagonal to
    for (int i = 0; i < size_n; i++) {
        similarity_matrix[i][i] = 0;
    }

    std::cout << "Minimal similarity: " << minimal_similarity << std::endl;

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
    long iteration = 0;

    while (changed && iteration < max_iteration) {
        iteration++;

        // Calculate responsibility matrix
#pragma omp parallel for default(none) shared(matrix_A, matrix_S, matrix_R, size_n)
        for (int i = 0; i < size_n; i++) {
            for (int k = 0; k < size_n; k++) {
                matrix_R[i][k] = matrix_S[i][k];

                double maximum = 0;
                for (int k_ = 0; k_ < size_n; k_++) {
                    maximum = std::max(maximum, matrix_A[i][k_] + matrix_S[i][k_]);
                }
                matrix_R[i][k] -= maximum;
            }
        }

        // Calculate availability matrix
#pragma omp parallel for default(none) shared(matrix_A, matrix_R, matrix_C, size_n)
        for (int i = 0; i < size_n; i++) {
            for (int k = 0; k < size_n; k++) {
                double sum_on_max = 0;
                for (int i_ = 0; i_ < size_n; i_++) {
                    if (i_ != i && i_ != k) {
                        sum_on_max = std::max(sum_on_max, matrix_R[i_][k]);
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
#pragma omp parallel for default(none) shared(matrix_A, matrix_R, matrix_C, size_n)
        for (int i = 0; i < size_n; i++) {
            for (int k = 0; k < size_n; k++) {
                const double new_value = matrix_A[i][k] + matrix_R[i][k];
                const double old_value = matrix_C[i][k];
                changed = changed || (new_value != old_value);
                matrix_C[i][k] = new_value;
            }
        }
    }
    return matrix_C;
}

void create_clusters(const std::vector<std::vector<double> > &matrix_C) {
    const size_t size_n = matrix_C.size();

    for (size_t i = 0; i < size_n; i++) {
        double max_value = INT_MIN;
        double max_index = -1;

        for (size_t j = 0; j < matrix_C[i].size(); j++) {
            if (matrix_C[i][j] > max_value) {
                max_value = matrix_C[i][j];
                max_index = static_cast<double>(j);
            }
        }


        if (max_index != -1) {
            std::cout << "Row " << i << " belongs to cluster " << max_index << std::endl;
        } else {
            std::cout << "Row " << i << " has no valid cluster." << std::endl;
        }
    }
}

int main() {
    const std::string mnist_file_train = "../project_2/mnist/mnist_train.csv";
    const std::string mnist_file_test = "../project_2/mnist/mnist_test.csv";
    const std::vector<std::string> mnist_dataset = merge_two_csv(mnist_file_train, mnist_file_test);

    const std::vector<std::string> five_participant_dataset = read_csv_file(
        "../project_2/five_participants.csv");
    const std::vector<std::vector<double> > five_participant_matrix = tokenize_csv(five_participant_dataset);
    create_clusters(five_participant_matrix);

    const std::vector<std::vector<double> > mnist_matrix = tokenize_csv(mnist_dataset);
    create_clusters(mnist_matrix);


    return 0;
}
