#include "StreamingUnit.hpp"
#include "instructions.hpp"
#include <cstddef> //std::size_t
#include <iostream>
using std::size_t;


int main()
{
    StreamingUnit SU;

    constexpr size_t size_in_i = 5;
    constexpr size_t size_in_j = 5;
    constexpr size_t stride_in = 1;
    constexpr size_t size_out = 5;
    constexpr size_t stride_out = 1;
    using ComputationType = float;
    using StorageType = std::uint32_t;

    ComputationType input_1[size_in_i][size_in_j];
    ComputationType input_2[size_in_i][size_in_j];
    ComputationType output[size_out][size_out];

    // Init input values
    ComputationType v = -5;
    for (size_t i = 0; i < size_in_i; i++) {
        for (size_t j = 0; j < size_in_j; j++) {
            input_1[i][j] = v;
            input_2[i][j] = v++;
        }
    }


    configureSimpleLD<StorageType>(SU, 0, input_1, 25,  stride_in);
    configureSimpleLD<StorageType>(SU, 1, input_2, 25,  stride_in);
    configureSimpleST<StorageType>(SU, 2,  output, 25, stride_out);

    so_a_add_fp(SU, 2, 0, 1);
    so_a_add_fp(SU, 2, 0, 1);
    // so_a_add_sg(SU, 2, 0, 1);
    // so_a_add_sg(SU, 2, 0, 1);
    // mv(SU, 2, 0);

    for (size_t i = 0; i < size_out; i++) {
        for (size_t j = 0; j < size_out; j++) {
            std::cout << output[i][j] << ", ";
        }
        std::cout << '\n';
    }
}