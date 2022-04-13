auto idxDest = insn.uve_comp_dest();
auto idxSrc1 = insn.uve_comp_src1();
auto idxSrc2 = insn.uve_comp_src2();


std::visit([](auto& dest, auto& src1, auto& src2) {
      /* Streams can only output/input values if they are in the running
       * status */
      const bool runningcheck = src1.getStatus() != RegisterStatus::Finished &&
        src2.getStatus() != RegisterStatus::Finished;
      assert_msg("Stream was not configured to be running", runningcheck);
      if (dest.getType() == RegisterType::Store) {
        assert_msg("Store destination stream was not running",
          dest.getStatus() != RegisterStatus::Finished);
      }
      /* Each stream's elements must have the same width for content to be
       * operated on */
      const bool src1Check = src1.getType() == RegisterType::Load || src1.getType() == RegisterType::Duplicate;
      const bool src2Check = src2.getType() == RegisterType::Load || src2.getType() == RegisterType::Duplicate;
      if (src1Check && src2Check) {
        assert_msg("Given streams have different widths",
          src1.getElementsWidth() == src2.getElementsWidth());
      }
      /* We can only operate on the first available values of the stream */
      auto elements1 = src1.getElements(true);
      auto elements2 = src2.getElements(true);
      auto validElementsIndex = std::min(elements1.size(), elements2.size());
      /* Grab used types for storage and operation */
      using StorageType = typename std::remove_reference_t<decltype(src1)>::ElementsType;
      using OperationType = ComputationTypeFp<StorageType>;
      decltype(dest.getElements(false)) out;
      for (size_t i = 0; i < validElementsIndex; i++) {
        auto e1 = readAS<OperationType>(elements1.at(i));
        auto e2 = readAS<OperationType>(elements2.at(i));
        auto value = e1 + e2;
        auto outPreStore = readAS<StorageType>(value);
        out.push_back(outPreStore);
      }
      dest.setElements(true, out);
    },
      P.SU.registers[idxDest],
      P.SU.registers[idxSrc1],
      P.SU.registers[idxSrc2]);
