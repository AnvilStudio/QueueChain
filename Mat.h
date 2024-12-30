////////////////////////////////////////////////
/// Basic matrix math and generation stuff to //
/// test the worker thread                    //
////////////////////////////////////////////////

#pragma once
#include <iostream>
#include <vector>
#include <random>

using mat = std::vector<std::vector<int>>;

inline void MultiplyMatrices(const mat& A,
    const std::vector<std::vector<int>>& B,
    mat& C) {
    std::cout << "Hello 2!\n";
    size_t M = A.size();
    size_t N = A[0].size();
    size_t P = B[0].size();

    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < P; ++j) {
            C[i][j] = 0; // Initialize result cell
            for (size_t k = 0; k < N; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

inline mat GenerateRandomMatrix(size_t rows, size_t cols, int max_value = 100) {
    mat matrix(rows, std::vector<int>(cols));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, max_value);

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            matrix[i][j] = dis(gen);
        }
    }

    return matrix;
}

inline std::ostream& operator<<(std::ostream& os, const mat& matrix) {
    for (const auto& row : matrix) {
        for (const auto& element : row) {
            os << element << " ";
        }
        os << "\n"; // Newline after each row
    }
    return os;
}

inline std::string to_string(mat& _mat)
{
    std::stringstream ss;
    for (const auto& row : _mat) {
        for (const auto& element : row) {
            ss << element << " ";
        }
        ss << "\n"; // Newline after each row
    }
    return ss.str();
}