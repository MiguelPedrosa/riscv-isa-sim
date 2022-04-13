#define readRegAS(T, reg) static_cast<T>( READ_REG(reg) )

auto streamReg = insn.uve_conf_rgD();
auto baseReg = insn.uve_conf_rgBase();
auto sizeReg = insn.uve_conf_rgN();
auto strideReg = insn.uve_conf_rgStride();

auto src = readRegAS(reg_t, baseReg);
auto size = readRegAS(std::size_t, sizeReg);
auto stride = readRegAS(std::size_t, strideReg);


operateRegister(P.SU, streamReg, [=](auto& reg) {
  reg.addDimension(Dimension(src, size, stride));
  reg.endConfiguration();
});
