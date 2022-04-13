#define readRegAS(T, reg) static_cast<T>( READ_REG(reg) )

auto streamReg = insn.uve_conf_rgD();
/* For now, this value is not used */
auto linkedReg = insn.uve_conf_rgBase();
auto displacementReg = insn.uve_conf_rgStride();
auto displacement = readRegAS(std::size_t, displacementReg);

operateRegister(P.SU, streamReg, [=](auto& reg) {
    reg.addModifier({ Modifier::Type::Static, Modifier::Target::Size, Modifier::Behaviour::Increment, displacement });
});

