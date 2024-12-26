#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

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

int main() {
    const std::string filename = "../project_1/Y-10_t.txt";
    std::vector<std::string> data = load_file(filename);

    if (data.empty()) {
        return 1;
    }

    int number_of_rows = std::stoi(data[0]);

    std::vector<int> sizes;
    std::istringstream sizes_stream(data[1]);
    int size;
    while (sizes_stream >> size) {
        sizes.push_back(size);
    }

    std::vector<std::vector<int> > matrix(number_of_rows, std::vector<int>(number_of_rows, 0));
    for (int i = 0; i < number_of_rows; ++i) {
        std::istringstream row_stream(data[i + 2]);
        for (int j = 0; j < number_of_rows; ++j) {
            row_stream >> matrix[i][j];
        }
    }

    std::cout << "Number of rows and columns: " << number_of_rows << std::endl;
    std::cout << "Sizes: ";
    for (const auto &s: sizes) {
        std::cout << s << " ";
    }
    std::cout << std::endl;

    std::cout << "Matrix:" << std::endl;
    for (const auto &row: matrix) {
        for (const auto &elem: row) {
            std::cout << elem << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
