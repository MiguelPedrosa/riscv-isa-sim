#include "StreamingUnit.hpp"
#include "instructions.hpp"
#include <cstddef> //std::size_t
#include <iostream>
using std::size_t;

int main() {
  StreamingUnit SU;

  constexpr size_t size = 5;
  constexpr size_t stride = 1;
  using ComputationType = float;
  using StorageType = std::uint32_t;

  ComputationType x[size][size];
  ComputationType y[size][size];
  ComputationType A = 3.f;

  // Init input values
  ComputationType v = -5;
  for (size_t i = 0; i < size; i++) {
    for (size_t j = 0; j < size; j++) {
      x[i][j] = v;
      y[i][j] = v++;
    }
  }

  ss_ld<StorageType>(SU, 1, x, size * size, stride);
  ss_ld<StorageType>(SU, 2, y, size * size, stride);
  ss_st<StorageType>(SU, 3, y, size * size, stride);
  so_v_dp<StorageType>(SU, 4, A);

  while (so_b_nc(1)) {
    so_a_mul_fp(SU, 5, 1, 4);
    so_a_add_fp(SU, 3, 2, 5);
  }

  for (size_t i = 0; i < size; i++) {
    for (size_t j = 0; j < size; j++) {
      std::cout << y[i][j] << ", ";
    }
    std::cout << '\n';
  }
}