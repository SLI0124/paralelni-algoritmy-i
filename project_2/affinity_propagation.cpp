#include <fstream>
#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> merge_two_csv(const std::string &filename1, const std::string &filename2) {
    std::vector<std::string> data_file1;
    std::vector<std::string> data_file2;
    std::vector<std::string> result;

    std::ifstream file1(filename1);
    if (!file1.is_open()) {
        std::cerr << "Error: Could not open the file " << filename1 << std::endl;
        return data_file1;
    }

    std::ifstream file2(filename2);
    if (!file2.is_open()) {
        std::cerr << "Error: Could not open the file " << filename2 << std::endl;
        return data_file2;
    }

    std::string line;

    // Read the header from file1
    std::getline(file1, line);
    data_file1.push_back(line);

    // Read the rest of file1
    while (std::getline(file1, line)) {
        data_file1.push_back(line);
    }
    file1.close();

    // Read the header from file2 (skip it)
    std::getline(file2, line);

    // Read the rest of file2
    while (std::getline(file2, line)) {
        data_file2.push_back(line);
    }
    file2.close();

    // Append the contents of both files
    result.insert(result.end(), data_file1.begin(), data_file1.end());
    result.insert(result.end(), data_file2.begin(), data_file2.end());

    std::cout << "Size of file1: " << data_file1.size() << std::endl;
    std::cout << "Size of file2: " << data_file2.size() << std::endl;
    std::cout << "Size of merged file: " << result.size() << std::endl;

    return result;
}

std::vector<std::string> read_csv_file(const std::string &filename) {
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

int main() {
    const std::string mnist_file_train = "../project_2/mnist/mnist_train.csv";
    const std::string mnist_file_test = "../project_2/mnist/mnist_test.csv";
    const std::vector<std::string> mnist_dataset = merge_two_csv(mnist_file_train, mnist_file_test);

    const std::vector<std::string> five_participant_dataset = read_csv_file("../project_2/five_participants.csv");

    std::cout << mnist_dataset[0] << std::endl << std::endl << mnist_dataset[1] << std::endl << std::endl << std::endl;

    std::cout << five_participant_dataset[0] << std::endl << std::endl << five_participant_dataset[1] << std::endl;

    return 0;
}
