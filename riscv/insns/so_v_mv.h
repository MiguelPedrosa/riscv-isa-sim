auto idxDest = insn.uve_comp_dest();
auto idxSrc = insn.uve_conf_rgBase();

std::visit([](auto& dest, auto& src) {
    /* Streams can only output/input values if they are in the running status */
    const bool runningcheck = src.getStatus() != RegisterStatus::Finished;
    assert_msg("Stream was not configured to be running", runningcheck);
    if (dest.getType() == RegisterType::Store) {
      assert_msg("Store destination stream was not running",
        dest.getStatus() != RegisterStatus::Finished);
    }
    /* We can only operate on the first available values of the stream */
    auto elements = src.getElements(true);
    /* Grab used types for storage and operation */
    using StorageType = typename std::remove_reference_t<decltype(src)>::ElementsType;
    using OperationType = ComputationTypeFp<StorageType>;
    decltype(dest.getElements(false)) out;
    for (size_t i = 0; i < elements.size(); i++) {
      auto e = readAS<OperationType>(elements.at(i));
      auto outPreStore = readAS<StorageType>(e);
      out.push_back(outPreStore);
    }
    dest.setElements(true, out);
  },
  P.SU.registers[idxDest],
  P.SU.registers[idxSrc]);
