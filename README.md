# SailToCpp

This is a transpiler that takes sail code and generates cpp functors specific to the Pegasus repository. 


As we know that riscv extensions are published in the sail specification language, and we do the painstaking work of writing it to something that is used in our simulators. This is a wastage of precious engineer time that could be used somewhere else. When riscv foundation is already giving use in a language how extensions should behave why not use that to our advantage! 


This tool takes sail code like : 


```sail
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
```

and transpiles them into something like this : 

```cpp
#pragma once
#include <bit>
#include <cstdint>

namespace pegasus
{
template <typename XLEN> struct SHA256SIG0Op
{
    XLEN operator()(XLEN rs1, XLEN /*rs2*/, uint32_t /*imm*/) const
      {
            const uint32_t inb = static_cast<uint32_t>(rs1);
            uint32_t result = ((std::rotr<uint32_t>(inb, 7) ^ std::rotr<uint32_t>(inb, 18)) ^ (inb >> 3));
            result = static_cast<XLEN>(static_cast<int32_t>(result));
            return result;
     }
   };

template <typename XLEN> struct RorOp
{
    XLEN operator()(XLEN rs1, XLEN rs2, uint32_t /*imm*/) const
      {
            const XLEN shamt = ((xlen == 32) ? ((rs2 >> 0) & 0x1f) : ((rs2 >> 0) & 0x3f));
            XLEN result = ((rs1 >> shamt) | (rs1 << (xlen - shamt)));
            return result;
     }
   };

}
```
