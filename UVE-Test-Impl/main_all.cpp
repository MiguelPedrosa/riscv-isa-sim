#include "StreamingUnit.hpp"
#include "instructions.hpp"
#include <cstddef> //std::size_t
#include <iostream>
#include <variant>
using std::size_t;

int main()
{
  StreamingUnit SU;

  constexpr size_t size = 8;
  using ComputationType = double;
  using StorageType = std::uint64_t;

  ComputationType x[size][size];
  // ComputationType y[size][size];
  // ComputationType A = 3.f;

  // Init input values
  ComputationType v = 0;
  for (size_t i = 0; i < size; i++) {
    for (size_t j = 0; j < size; j++) {
      x[i][j] = v++;
      // y[i][j] = v++;
    }
  }

  // ss_sta_ld<StorageType>(SU, 1, x,    1,    1);
  //   ss_app_mod_size_inc(SU, 1);
  //    ss_end<StorageType>(SU, 1, 0, size, size);
  // ss_sta_ld<StorageType>(SU, 2, y,    1,    1);
  //   ss_app_mod_size_inc(SU, 2);
  //    ss_end<StorageType>(SU, 2, 0, size, size);
  // ss_sta_st<StorageType>(SU, 3, y,    1,    1);
  //   ss_app_mod_size_inc(SU, 3);
  //    ss_end<StorageType>(SU, 3, 0, size, size);

  ss_sta_ld<StorageType>(SU, 1, x, 6, 1);
    // ss_app_mod_size_inc(SU, 1, 1);
    ss_cfg_vec(SU, 1);
      ss_end(SU, 1, 0, 6, size);

  modifyRegister(SU, 1, [](auto& reg) {
    while (!reg.hasStreamFinished()) {
      auto values = reg.getElements(true);
      std::for_each(values.begin(), values.end(), [](auto v) {
        std::cout << readAS<ComputationType>(v) << ", ";
      });
      std::cout << '\n';
      if (reg.isEndOfDimensionOfDim(0)) {
        reg.clearEndOfDimensionOfDim(0);
        std::cout << "Cleared eod for dim 0\n";
      }
    }
  });
  // ss_ld<StorageType>(SU, 2, y, size, 1);
  // ss_st<StorageType>(SU, 3, y, size, 1);
  // so_v_dp<StorageType>(SU, 4, A);

  // while (so_b_nc(1)) {
  //   so_a_mul_fp(SU, 5, 1, 4);
  //   so_a_add_fp(SU, 3, 2, 5);
  // }

  // for (size_t i = 0; i < size; i++) {
  //   for (size_t j = 0; j < size; j++) {
  //     std::cout << y[i][j] << ", ";
  //   }
  //   std::cout << '\n';
  // }
}