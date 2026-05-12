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

template <typename XLEN> struct SHA256SIG1Op
{
    XLEN operator()(XLEN rs1, XLEN /*rs2*/, uint32_t /*imm*/) const
      {
            const uint32_t inb = static_cast<uint32_t>(rs1);
            uint32_t result = ((std::rotr<uint32_t>(inb, 17) ^ std::rotr<uint32_t>(inb, 19)) ^ (inb >> 10));
            result = static_cast<XLEN>(static_cast<int32_t>(result));
            return result;
     }
   };

template <typename XLEN> struct SHA256SUM0Op
{
    XLEN operator()(XLEN rs1, XLEN /*rs2*/, uint32_t /*imm*/) const
      {
            const uint32_t inb = static_cast<uint32_t>(rs1);
            uint32_t result = ((std::rotr<uint32_t>(inb, 2) ^ std::rotr<uint32_t>(inb, 13)) ^ std::rotr<uint32_t>(inb, 22));
            result = static_cast<XLEN>(static_cast<int32_t>(result));
            return result;
     }
   };

template <typename XLEN> struct SHA256SUM1Op
{
    XLEN operator()(XLEN rs1, XLEN /*rs2*/, uint32_t /*imm*/) const
      {
            const uint32_t inb = static_cast<uint32_t>(rs1);
            uint32_t result = ((std::rotr<uint32_t>(inb, 6) ^ std::rotr<uint32_t>(inb, 11)) ^ std::rotr<uint32_t>(inb, 25));
            result = static_cast<XLEN>(static_cast<int32_t>(result));
            return result;
     }
   };

template <typename XLEN> struct SHA512SIG0HOp
{
    XLEN operator()(XLEN rs1, XLEN rs2, uint32_t /*imm*/) const
      {
            const XLEN result = static_cast<XLEN>(static_cast<int32_t>((((((rs1 >> 1) ^ (rs1 >> 7)) ^ (rs1 >> 8)) ^ (rs2 << 31)) ^ (rs2 << 24))));
            return result;
     }
   };

template <typename XLEN> struct SHA512SIG0LOp
{
    XLEN operator()(XLEN rs1, XLEN rs2, uint32_t /*imm*/) const
      {
            const XLEN result = static_cast<XLEN>(static_cast<int32_t>(((((((rs1 >> 1) ^ (rs1 >> 7)) ^ (rs1 >> 8)) ^ (rs2 << 31)) ^ (rs2 << 25)) ^ (rs2 << 24))));
            return result;
     }
   };

template <typename XLEN> struct SHA512SIG1HOp
{
    XLEN operator()(XLEN rs1, XLEN rs2, uint32_t /*imm*/) const
      {
            const XLEN result = static_cast<XLEN>(static_cast<int32_t>((((((rs1 << 3) ^ (rs1 >> 6)) ^ (rs1 >> 19)) ^ (rs2 >> 29)) ^ (rs2 << 13))));
            return result;
     }
   };

template <typename XLEN> struct SHA512SIG1LOp
{
    XLEN operator()(XLEN rs1, XLEN rs2, uint32_t /*imm*/) const
      {
            const XLEN result = static_cast<XLEN>(static_cast<int32_t>(((((((rs1 << 3) ^ (rs1 >> 6)) ^ (rs1 >> 19)) ^ (rs2 >> 29)) ^ (rs2 << 26)) ^ (rs2 << 13))));
            return result;
     }
   };

template <typename XLEN> struct SHA512SUM0ROp
{
    XLEN operator()(XLEN rs1, XLEN rs2, uint32_t /*imm*/) const
      {
            const XLEN result = static_cast<XLEN>(static_cast<int32_t>(((((((rs1 << 25) ^ (rs1 << 30)) ^ (rs1 >> 28)) ^ (rs2 >> 7)) ^ (rs2 >> 2)) ^ (rs2 << 4))));
            return result;
     }
   };

template <typename XLEN> struct SHA512SUM1ROp
{
    XLEN operator()(XLEN rs1, XLEN rs2, uint32_t /*imm*/) const
      {
            const XLEN result = static_cast<XLEN>(static_cast<int32_t>(((((((rs1 << 23) ^ (rs1 >> 14)) ^ (rs1 >> 18)) ^ (rs2 >> 9)) ^ (rs2 << 18)) ^ (rs2 << 14))));
            return result;
     }
   };

template <typename XLEN> struct SHA512SIG0Op
{
    XLEN operator()(XLEN rs1, XLEN /*rs2*/, uint32_t /*imm*/) const
      {
            const XLEN result = ((std::rotr<uint64_t>(rs1, 1) ^ std::rotr<uint64_t>(rs1, 8)) ^ (rs1 >> 7));
            return result;
     }
   };

template <typename XLEN> struct SHA512SIG1Op
{
    XLEN operator()(XLEN rs1, XLEN /*rs2*/, uint32_t /*imm*/) const
      {
            const XLEN result = ((std::rotr<uint64_t>(rs1, 19) ^ std::rotr<uint64_t>(rs1, 61)) ^ (rs1 >> 6));
            return result;
     }
   };

template <typename XLEN> struct SHA512SUM0Op
{
    XLEN operator()(XLEN rs1, XLEN /*rs2*/, uint32_t /*imm*/) const
      {
            const XLEN result = ((std::rotr<uint64_t>(rs1, 28) ^ std::rotr<uint64_t>(rs1, 34)) ^ std::rotr<uint64_t>(rs1, 39));
            return result;
     }
   };

template <typename XLEN> struct SHA512SUM1Op
{
    XLEN operator()(XLEN rs1, XLEN /*rs2*/, uint32_t /*imm*/) const
      {
            const XLEN result = ((std::rotr<uint64_t>(rs1, 14) ^ std::rotr<uint64_t>(rs1, 18)) ^ std::rotr<uint64_t>(rs1, 41));
            return result;
     }
   };

}
