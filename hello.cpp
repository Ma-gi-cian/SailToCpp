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
   }

template <typename XLEN> struct RorOp
{
    XLEN operator()(XLEN rs1, XLEN rs2, uint32_t /*imm*/) const
      {
            const XLEN shamt = ((xlen == 32) ? ((rs2 >> 0) & 0x1f) : ((rs2 >> 0) & 0x3f));
            XLEN result = ((rs1 >> shamt) | (rs1 << (xlen - shamt)));
            return result;
     }
   }

}
