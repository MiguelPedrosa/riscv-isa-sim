#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include "StreamingUnit.hpp"
#include "helpers.hpp"
#include <variant>

template<typename Operation>
void modifyRegister(StreamingUnit& SU, size_t index, Operation&& op) {
  assert_msg("Tried to use a register index higher than available registers",
             index < StreamingUnit::registerCount);
  std::visit([op = std::move(op)](auto& reg) {
      op(reg);
  }, SU.registers.at(index));
}


template <typename T>
void ss_ld(StreamingUnit &SU, size_t regIdx, void *src, size_t size,
           size_t stride) {
  SU.registers[regIdx] = makeStreamRegister<T>(RegisterType::Load);
  modifyRegister(SU, regIdx, [=](auto &reg) {
    reg.startConfiguration({src, size, stride});
    reg.endConfiguration();
  });
}

template <typename T>
void ss_sta_ld(StreamingUnit &SU, size_t regIdx, void *src, size_t size,
           size_t stride) {
  SU.registers[regIdx] = makeStreamRegister<T>(RegisterType::Load);
  modifyRegister(SU, regIdx, [=](auto &reg) {
    reg.startConfiguration({src, size, stride});
  });
}

void ss_end(StreamingUnit &SU, size_t regIdx, void *src, size_t size,
           size_t stride) {
  modifyRegister(SU, regIdx, [=](auto &reg) {
    reg.addDimension({src, size, stride});
    reg.endConfiguration();
  });
}

void ss_app_mod_size_inc(StreamingUnit &SU, size_t regIdx, std::size_t displacement) {
  modifyRegister(SU, regIdx, [=](auto &reg) {
    reg.addModifier({Modifier::Type::Static, Modifier::Target::Size, Modifier::Behaviour::Increment, displacement });
  });
}

void ss_cfg_vec(StreamingUnit &SU, size_t regIdx) {
  modifyRegister(SU, regIdx, [=](auto &reg) {
    reg.addModifier({Modifier::Type::CfgVec});
  });
}




// template <typename T>
// void ss_st(StreamingUnit &SU, size_t regIdx, void *src, size_t size,
//            size_t stride) {
//   assert_msg("Tried to use a register index higher than available registers",
//              regIdx < StreamingUnit::registerCount);
//   SU.registers[regIdx] = makeStreamRegister<T>();
//   modifyRegister(SU, regIdx, [=](auto &reg) {
//     reg.type = RegisterType::Store;
//     reg.startConfiguration({src, size, stride});
//     reg.endConfiguration();
//   });
// }

// void so_v_mv(StreamingUnit &SU, int idxDest, int idxSrc) {
//   std::visit(
//       [](auto &dest, auto &src) {
//         /* Streams can only output/input values if they are in the running
//          * status */
//         const bool runningcheck = src.status != RegisterStatus::Finished;
//         assert_msg("Stream was not configured to be running", runningcheck);
//         if (dest.type == RegisterType::Store) {
//           assert_msg("Store destination stream was not running",
//                      dest.status != RegisterStatus::Finished);
//         }
//         /* We can only operate on the first available values of the stream */
//         src.updateStreamValues();
//         auto &elements = src.elements;
//         using StorageType =
//             typename std::remove_reference_t<decltype(src)>::elementsType;
//         using OperationType =
//             StorageToOperation_t<sizeof(StorageType), UnsignedInt>;
//         for (size_t i = 0; i < elements.size(); i++) {
//           auto e = *reinterpret_cast<OperationType *>(&elements.at(i));
//           // Type e = elements.at(i);
//           auto value = e;
//           dest.elements.push_back(*reinterpret_cast<StorageType *>(&value));
//         }
//         dest.updateStreamValues();
//       },
//       SU.registers[idxDest], SU.registers[idxSrc]);
// }

// void so_a_add_fp(StreamingUnit &SU, int idxDest, int idxSrc1, int idxSrc2) {
//   std::visit(
//       [](auto &dest, auto &src1, auto &src2) {
//         /* Streams can only output/input values if they are in the running
//          * status */
//         const bool runningcheck = src1.status != RegisterStatus::Finished &&
//                                   src2.status != RegisterStatus::Finished;
//         assert_msg("Stream was not configured to be running", runningcheck);
//         if (dest.type == RegisterType::Store) {
//           assert_msg("Store destination stream was not running",
//                      dest.status != RegisterStatus::Finished);
//         }
//         /* Each stream's elements must have the same width for content to be
//          * operated on */
//         assert_msg("Given streams have different widths",
//                    src1.elementsWidth == src2.elementsWidth);
//         /* We can only operate on the first available values of the stream */
//         src1.updateStreamValues();
//         src2.updateStreamValues();
//         auto &elements1 = src1.elements;
//         auto &elements2 = src2.elements;
//         auto validElementsIndex = std::min(elements1.size(), elements2.size());
//         /* Grab used types for storage and operation */
//         using StorageType =
//             typename std::remove_reference_t<decltype(src1)>::elementsType;
//         using OperationType =
//             std::conditional_t<std::is_same_v<std::uint32_t, StorageType>,
//                                float, double>;
//         for (size_t i = 0; i < validElementsIndex; i++) {
//           auto e1 = readAS<OperationType>(elements1.at(i));
//           auto e2 = readAS<OperationType>(elements2.at(i));
//           auto value = e1 + e2;
//           dest.elements.push_back(readAS<StorageType>(value));
//         }
//         dest.updateStreamValues();
//       },
//       SU.registers[idxDest], SU.registers[idxSrc1], SU.registers[idxSrc2]);
// }
// void so_a_mul_fp(StreamingUnit &SU, int idxDest, int idxSrc1, int idxSrc2) {
//   std::visit(
//       [](auto &dest, auto &src1, auto &src2) {
//         /* Streams can only output/input values if they are in the running
//          * status */
//         const bool runningcheck = src1.status != RegisterStatus::Finished &&
//                                   src2.status != RegisterStatus::Finished;
//         assert_msg("Stream was not configured to be running", runningcheck);
//         if (dest.type == RegisterType::Store) {
//           assert_msg("Store destination stream was not running",
//                      dest.status != RegisterStatus::Finished);
//         }
//         /* Each stream's elements must have the same width for content to be
//          * operated on */
//         assert_msg("Given streams have different widths",
//                    src1.elementsWidth == src2.elementsWidth);
//         /* We can only operate on the first available values of the stream */
//         src1.updateStreamValues();
//         src2.updateStreamValues();
//         auto &elements1 = src1.elements;
//         auto &elements2 = src2.elements;
//         auto validElementsIndex = std::min(elements1.size(), elements2.size());
//         /* Grab used types for storage and operation */
//         using StorageType =
//             typename std::remove_reference_t<decltype(src1)>::elementsType;
//         using OperationType =
//             std::conditional_t<std::is_same_v<std::uint32_t, StorageType>,
//                                float, double>;
//         for (size_t i = 0; i < validElementsIndex; i++) {
//           auto e1 = readAS<OperationType>(elements1.at(i));
//           auto e2 = readAS<OperationType>(elements2.at(i));
//           auto value = e1 + e2;
//           dest.elements.push_back(readAS<StorageType>(value));
//         }
//         dest.updateStreamValues();
//       },
//       SU.registers[idxDest], SU.registers[idxSrc1], SU.registers[idxSrc2]);
// }

#endif // INSTRUCTIONS_HPP