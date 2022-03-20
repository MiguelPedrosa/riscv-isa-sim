auto streamReg = insn.uve_conf_rgD();
auto baseReg = insn.uve_conf_rgBase();
auto sizeReg = insn.uve_conf_rgN();
auto strideReg = insn.uve_conf_rgStride();

printf("UVE stream reg idx: %ld\n", streamReg);
printf("UVE base   reg idx: %ld; content: %p\n", baseReg, READ_REG(baseReg));
printf("UVE size   reg idx: %ld; content: %ld\n", sizeReg, READ_REG(sizeReg));
printf("UVE stride reg idx: %ld; content: %ld\n", strideReg, READ_REG(strideReg));

