#define readRegAS(T, reg) static_cast<T>( READ_REG(reg) )

auto streamReg = insn.uve_conf_rgD();
auto baseReg = insn.uve_conf_rgBase();
auto sizeReg = insn.uve_conf_rgN();
auto strideReg = insn.uve_conf_rgStride();

auto base = readRegAS(reg_t, baseReg);
auto size = readRegAS(std::int32_t, sizeReg);
auto stride = readRegAS(std::int32_t, strideReg);

printf("Runnning ss.ld.w instruction\n");
printf("UVE stream reg idx: %ld\n", streamReg);
printf("UVE base   reg idx: %ld; content: %p\n", baseReg, (void*) READ_REG(baseReg));
printf("UVE size   reg idx: %ld; content: %d\n", sizeReg, size);
printf("UVE stride reg idx: %ld; content: %d\n", strideReg, stride);

for (std::int32_t i = 0; i < size; i += stride) {
    std::int32_t offset = i * sizeof(std::int32_t);
    uint32_t v = MMU.load_uint32(base + offset);
    printf("arr[%d]: %f\n", i, *static_cast<float*>(static_cast<void*>(&v)));
}
