function clause execute (SHA256SIG0(rs1,rd)) = {
  let inb    : bits(32) = X(rs1)[31..0];
  let result : bits(32) = ror32(inb,  7) ^ ror32(inb, 18) ^ (inb >>  3);
  X(rd)      = EXTS(result);
  RETIRE_SUCCESS
}

function clause execute(ror(rs1, rs2)) = {
  let shamt = if   xlen == 32
              then X(rs2)[4..0]
              else X(rs2)[5..0];
  let result = (X(rs1) >> shamt) | (X(rs1) << (xlen - shamt));
  X(rd) = result;
  RETIRE_SUCCESS
}
