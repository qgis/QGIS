// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.02.21

#pragma once

#include <Mathematics/BitHacks.h>
#include <algorithm>

// Support for unsigned integer arithmetic in BSNumber and BSRational.  The
// Curiously Recurring Template Paradigm is used to allow the UInteger
// types to share code without introducing virtual functions.

namespace gte
{
    template <typename UInteger>
    class UIntegerALU32
    {
    public:
        // Comparisons.  These are not generic.  They rely on their being
        // called when the two BSNumber arguments to BSNumber::operatorX()
        // are of the form 1.u*2^p and 1.v*2^p.  The comparisons apply to
        // 1.u and 1.v as unsigned integers with their leading 1-bits aligned.
        bool operator==(UInteger const& number) const
        {
            UInteger const& self = *(UInteger const*)this;
            int32_t numBits = self.GetNumBits();
            if (numBits != number.GetNumBits())
            {
                return false;
            }

            if (numBits > 0)
            {
                auto const& bits = self.GetBits();
                auto const& nBits = number.GetBits();
                int32_t const last = self.GetSize() - 1;
                for (int32_t i = last; i >= 0; --i)
                {
                    if (bits[i] != nBits[i])
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        bool operator!=(UInteger const& number) const
        {
            return !operator==(number);
        }

        bool operator< (UInteger const& number) const
        {
            UInteger const& self = *(UInteger const*)this;
            int32_t nNumBits = number.GetNumBits();
            auto const& nBits = number.GetBits();

            int32_t numBits = self.GetNumBits();
            if (numBits > 0 && nNumBits > 0)
            {
                // The numbers must be compared as if they are left-aligned
                // with each other.  We got here because we had
                // self = 1.u * 2^p and number = 1.v * 2^p.  Although they
                // have the same exponent, it is possible that
                // 'self < number' but 'numBits(1u) > numBits(1v)'.  Compare
                // the bits one 32-bit block at a time.
                auto const& bits = self.GetBits();
                int bitIndex0 = numBits - 1;
                int bitIndex1 = nNumBits - 1;
                int block0 = bitIndex0 / 32;
                int block1 = bitIndex1 / 32;
                int numBlockBits0 = 1 + (bitIndex0 % 32);
                int numBlockBits1 = 1 + (bitIndex1 % 32);
                uint64_t n0shift = bits[block0];
                uint64_t n1shift = nBits[block1];
                while (block0 >= 0 && block1 >= 0)
                {
                    // Shift the bits in the leading blocks to the high-order bit.
                    uint32_t value0 = (uint32_t)((n0shift << (32 - numBlockBits0)) & 0x00000000FFFFFFFFull);
                    uint32_t value1 = (uint32_t)((n1shift << (32 - numBlockBits1)) & 0x00000000FFFFFFFFull);

                    // Shift bits in the next block (if any) to fill the current
                    // block.
                    if (--block0 >= 0)
                    {
                        n0shift = bits[block0];
                        value0 |= (uint32_t)((n0shift >> numBlockBits0) & 0x00000000FFFFFFFFull);
                    }
                    if (--block1 >= 0)
                    {
                        n1shift = nBits[block1];
                        value1 |= (uint32_t)((n1shift >> numBlockBits1) & 0x00000000FFFFFFFFull);
                    }
                    if (value0 < value1)
                    {
                        return true;
                    }
                    if (value0 > value1)
                    {
                        return false;
                    }
                }
                return block0 < block1;
            }
            else
            {
                // One or both numbers are zero. The only time 'less than' is
                // 'true' is when 'number' is positive.
                return nNumBits > 0;
            }
        }

        bool operator<=(UInteger const& number) const
        {
            return operator<(number) || operator==(number);
        }

        bool operator> (UInteger const& number) const
        {
            return !operator<=(number);
        }

        bool operator>=(UInteger const& number) const
        {
            return !operator<(number);
        }

        // Arithmetic operations.  These are performed in-place; that is, the
        // result is stored in 'this' object.  The goal is to reduce the
        // number of object copies, much like the goal is for std::move.  The
        // Sub function requires the inputs to satisfy n0 > n1.
        void Add(UInteger const& n0, UInteger const& n1)
        {
            UInteger& self = *(UInteger*)this;
            int32_t n0NumBits = n0.GetNumBits();
            int32_t n1NumBits = n1.GetNumBits();

            // Add the numbers considered as positive integers.  Set the last
            // block to zero in case no carry-out occurs.
            int numBits = std::max(n0NumBits, n1NumBits) + 1;
            self.SetNumBits(numBits);
            self.SetBack(0);

            // Get the input array sizes.
            int32_t numElements0 = n0.GetSize();
            int32_t numElements1 = n1.GetSize();

            // Order the inputs so that the first has the most blocks.
            auto const& u0 = (numElements0 >= numElements1 ? n0.GetBits() : n1.GetBits());
            auto const& u1 = (numElements0 >= numElements1 ? n1.GetBits() : n0.GetBits());
            auto numElements = std::minmax(numElements0, numElements1);

            // Add the u1-blocks to u0-blocks.
            auto& bits = self.GetBits();
            uint64_t carry = 0, sum;
            int32_t i;
            for (i = 0; i < numElements.first; ++i)
            {
                sum = u0[i] + (u1[i] + carry);
                bits[i] = (uint32_t)(sum & 0x00000000FFFFFFFFull);
                carry = (sum >> 32);
            }

            // We have no more u1-blocks. Propagate the carry-out, if there is
            // one, or copy the remaining blocks if there is not.
            if (carry > 0)
            {
                for (/**/; i < numElements.second; ++i)
                {
                    sum = u0[i] + carry;
                    bits[i] = (uint32_t)(sum & 0x00000000FFFFFFFFull);
                    carry = (sum >> 32);
                }
                if (carry > 0)
                {
                    bits[i] = (uint32_t)(carry & 0x00000000FFFFFFFFull);
                }
            }
            else
            {
                for (/**/; i < numElements.second; ++i)
                {
                    bits[i] = u0[i];
                }
            }

            // Reduce the number of bits if there was not a carry-out.
            uint32_t firstBitIndex = (numBits - 1) % 32;
            uint32_t mask = (1 << firstBitIndex);
            if ((mask & self.GetBack()) == 0)
            {
                self.SetNumBits(--numBits);
            }
        }

        void Sub(UInteger const& n0, UInteger const& n1)
        {
            UInteger& self = *(UInteger*)this;
            int32_t n0NumBits = n0.GetNumBits();
            auto const& n0Bits = n0.GetBits();
            auto const& n1Bits = n1.GetBits();

            // Subtract the numbers considered as positive integers.  We know
            // that n0 > n1, so create a number n2 that has the same number of
            // bits as n0 and use two's-complement to generate -n2, and then
            // add n0 and -n2.  The result is nonnegative, so we do not need
            // to apply two's complement to a negative result to extract the
            // sign and absolute value.

            // Get the input array sizes.  We know
            // numElements0 >= numElements1.
            int32_t numElements0 = n0.GetSize();
            int32_t numElements1 = n1.GetSize();

            // Create the two's-complement number n2.  We know
            // n2.GetNumElements() is the same as numElements0.
            UInteger n2;
            n2.SetNumBits(n0NumBits);
            auto& n2Bits = n2.GetBits();
            int32_t i;
            for (i = 0; i < numElements1; ++i)
            {
                n2Bits[i] = ~n1Bits[i];
            }
            for (/**/; i < numElements0; ++i)
            {
                n2Bits[i] = ~0u;
            }

            // Now add 1 to the bit-negated result to obtain -n1.
            uint64_t carry = 1, sum;
            for (i = 0; i < numElements0; ++i)
            {
                sum = n2Bits[i] + carry;
                n2Bits[i] = (uint32_t)(sum & 0x00000000FFFFFFFFull);
                carry = (sum >> 32);
            }

            // Add the numbers as positive integers.  Set the last block to
            // zero in case no carry-out occurs.
            self.SetNumBits(n0NumBits + 1);
            self.SetBack(0);

            // Add the n0-blocks to n2-blocks.
            auto & bits = self.GetBits();
            for (i = 0, carry = 0; i < numElements0; ++i)
            {
                sum = n2Bits[i] + (n0Bits[i] + carry);
                bits[i] = (uint32_t)(sum & 0x00000000FFFFFFFFull);
                carry = (sum >> 32);
            }

            // Strip off the bits introduced by two's-complement.
            int32_t block;
            for (block = numElements0 - 1; block >= 0; --block)
            {
                if (bits[block] > 0)
                {
                    break;
                }
            }

            if (block >= 0)
            {
                self.SetNumBits(32 * block + BitHacks::GetLeadingBit(bits[block]) + 1);
            }
            else
            {
                // This block originally did not exist, only the if-block did.
                // During some testing for the RAEGC book, a crash occurred
                // where it appeared the block was needed. I added this block
                // to fix the problem, but had forgotten the precondition that
                // n0 > n1. Consequently, I did not look carefully at the
                // inputs and call stack to determine how this could have
                // happened. Trap this problem and analyze the call stack and
                // inputs that lead to this case if it happens again. The call
                // stack is started by BSNumber::SubIgnoreSign(...).
                LogWarning("The difference of the number is zero, which violates the precondition n0 > n1.");
                self.SetNumBits(0);
            }
        }

        void Mul(UInteger const& n0, UInteger const& n1)
        {
            UInteger& self = *(UInteger*)this;
            int32_t n0NumBits = n0.GetNumBits();
            int32_t n1NumBits = n1.GetNumBits();
            auto const& n0Bits = n0.GetBits();
            auto const& n1Bits = n1.GetBits();

            // The number of bits is at most this, possibly one bit smaller.
            int numBits = n0NumBits + n1NumBits;
            self.SetNumBits(numBits);
            auto& bits = self.GetBits();

            // Product of a single-block number with a multiple-block number.
            UInteger product;
            product.SetNumBits(numBits);
            auto& pBits = product.GetBits();

            // Get the array sizes.
            int32_t const numElements0 = n0.GetSize();
            int32_t const numElements1 = n1.GetSize();
            int32_t const numElements = self.GetSize();

            // Compute the product v = u0*u1.
            int32_t i0, i1, i2;
            uint64_t term, sum;

            // The case i0 == 0 is handled separately to initialize the
            // accumulator with u0[0]*v.  This avoids having to fill the bytes
            // of 'bits' with zeros outside the double loop, something that
            // can be a performance issue when 'numBits' is large.
            uint64_t block0 = n0Bits[0];
            uint64_t carry = 0;
            for (i1 = 0; i1 < numElements1; ++i1)
            {
                term = block0 * n1Bits[i1] + carry;
                bits[i1] = (uint32_t)(term & 0x00000000FFFFFFFFull);
                carry = (term >> 32);
            }
            if (i1 < numElements)
            {
                bits[i1] = (uint32_t)(carry & 0x00000000FFFFFFFFull);
            }

            for (i0 = 1; i0 < numElements0; ++i0)
            {
                // Compute the product p = u0[i0]*u1.
                block0 = n0Bits[i0];
                carry = 0;
                for (i1 = 0, i2 = i0; i1 < numElements1; ++i1, ++i2)
                {
                    term = block0 * n1Bits[i1] + carry;
                    pBits[i2] = (uint32_t)(term & 0x00000000FFFFFFFFull);
                    carry = (term >> 32);
                }
                if (i2 < numElements)
                {
                    pBits[i2] = (uint32_t)(carry & 0x00000000FFFFFFFFull);
                }

                // Add p to the accumulator v.
                carry = 0;
                for (i1 = 0, i2 = i0; i1 < numElements1; ++i1, ++i2)
                {
                    sum = pBits[i2] + (bits[i2] + carry);
                    bits[i2] = (uint32_t)(sum & 0x00000000FFFFFFFFull);
                    carry = (sum >> 32);
                }
                if (i2 < numElements)
                {
                    sum = pBits[i2] + carry;
                    bits[i2] = (uint32_t)(sum & 0x00000000FFFFFFFFull);
                }
            }

            // Reduce the number of bits if there was not a carry-out.
            uint32_t firstBitIndex = (numBits - 1) % 32;
            uint32_t mask = (1 << firstBitIndex);
            if ((mask & self.GetBack()) == 0)
            {
                self.SetNumBits(--numBits);
            }
        }

        // The shift is performed in-place; that is, the result is stored in
        // 'this' object.
        void ShiftLeft(UInteger const& number, int32_t shift)
        {
            UInteger& self = *(UInteger*)this;
            int32_t nNumBits = number.GetNumBits();
            auto const& nBits = number.GetBits();

            // Shift the 'number' considered as an odd positive integer.
            self.SetNumBits(nNumBits + shift);

            // Set the low-order bits to zero.
            auto& bits = self.GetBits();
            int32_t const shiftBlock = shift / 32;
            for (int32_t i = 0; i < shiftBlock; ++i)
            {
                bits[i] = 0;
            }

            // Get the location of the low-order 1-bit within the result.
            int32_t const numInElements = number.GetSize();
            int32_t const lshift = shift % 32;
            int32_t i, j;
            if (lshift > 0)
            {
                // The trailing 1-bits for source and target are at different
                // relative indices.  Each shifted source block straddles a
                // boundary between two target blocks, so we must extract the
                // subblocks and copy accordingly.
                int32_t const rshift = 32 - lshift;
                uint32_t prev = 0, curr;
                for (i = shiftBlock, j = 0; j < numInElements; ++i, ++j)
                {
                    curr = nBits[j];
                    bits[i] = (curr << lshift) | (prev >> rshift);
                    prev = curr;
                }
                if (i < self.GetSize())
                {
                    // The leading 1-bit of the source is at a relative index
                    // such that when you add the shift amount, that bit
                    // occurs in a new block.
                    bits[i] = (prev >> rshift);
                }
            }
            else
            {
                // The trailing 1-bits for source and target are at the same
                // relative index.  The shift reduces to a block copy.
                for (i = shiftBlock, j = 0; j < numInElements; ++i, ++j)
                {
                    bits[i] = nBits[j];
                }
            }
        }

        // The 'number' is even and positive.  It is shifted right to become
        // an odd number and the return value is the amount shifted.  The
        // operation is performed in-place; that is, the result is stored in
        // 'this' object.
        int32_t ShiftRightToOdd(UInteger const& number)
        {
            UInteger& self = *(UInteger*)this;
            auto const& nBits = number.GetBits();

            // Get the leading 1-bit.
            int32_t const numElements = number.GetSize();
            int32_t const numM1 = numElements - 1;
            int32_t firstBitIndex = 32 * numM1 + BitHacks::GetLeadingBit(nBits[numM1]);

            // Get the trailing 1-bit.
            int32_t lastBitIndex = -1;
            for (int32_t block = 0; block < numElements; ++block)
            {
                uint32_t value = nBits[block];
                if (value > 0)
                {
                    lastBitIndex = 32 * block + BitHacks::GetTrailingBit(value);
                    break;
                }
            }

            // The right-shifted result.
            self.SetNumBits(firstBitIndex - lastBitIndex + 1);
            auto& bits = self.GetBits();
            int32_t const numBlocks = self.GetSize();

            // Get the location of the low-order 1-bit within the result.
            int32_t const shiftBlock = lastBitIndex / 32;
            int32_t rshift = lastBitIndex % 32;
            if (rshift > 0)
            {
                int32_t const lshift = 32 - rshift;
                int32_t i, j = shiftBlock;
                uint32_t curr = nBits[j++];
                for (i = 0; j < numElements; ++i, ++j)
                {
                    uint32_t next = nBits[j];
                    bits[i] = (curr >> rshift) | (next << lshift);
                    curr = next;
                }
                if (i < numBlocks)
                {
                    bits[i] = (curr >> rshift);
                }
            }
            else
            {
                for (int32_t i = 0, j = shiftBlock; i < numBlocks; ++i, ++j)
                {
                    bits[i] = nBits[j];
                }
            }

            return rshift + 32 * shiftBlock;
        }

        // Add 1 to 'this', useful for rounding modes in conversions of
        // BSNumber and BSRational. The operation is performed in-place;
        // that is, the result is stored in 'this' object. The return value
        // is the amount shifted after the addition in order to obtain an
        // odd integer.
        int32_t RoundUp()
        {
            UInteger const& self = *(UInteger const*)this;
            UInteger rounded;
            rounded.Add(self, UInteger(1u));
            return ShiftRightToOdd(rounded);
        }

        // Get a block of numRequested bits starting with the leading 1-bit of
        // the nonzero number.  The returned number has the prefix stored in
        // the high-order bits.  Additional bits are copied and used by the
        // caller for rounding.  This function supports conversions from
        // 'float' and 'double'.  The input 'numRequested' is smaller than 64.
        uint64_t GetPrefix(int32_t numRequested) const
        {
            UInteger const& self = *(UInteger const*)this;
            auto const& bits = self.GetBits();

            // Copy to 'prefix' the leading 32-bit block that is nonzero.
            int32_t bitIndex = self.GetNumBits() - 1;
            int32_t blockIndex = bitIndex / 32;
            uint64_t prefix = bits[blockIndex];

            // Get the number of bits in the block starting with the leading
            // 1-bit.
            int32_t firstBitIndex = bitIndex % 32;
            int32_t numBlockBits = firstBitIndex + 1;

            // Shift the leading 1-bit to bit-63 of prefix.  We have consumed
            // numBlockBits, which might not be the entire budget.
            int32_t targetIndex = 63;
            prefix <<= targetIndex - firstBitIndex;
            numRequested -= numBlockBits;
            targetIndex -= numBlockBits;

            if (numRequested > 0 && --blockIndex >= 0)
            {
                // More bits are available.  Copy and shift the entire 32-bit
                // next block and OR it into the 'prefix'.  For 'float', we
                // will have consumed the entire budget.  For 'double', we
                // might have to get bits from a third block.
                uint64_t nextBlock = bits[blockIndex];
                nextBlock <<= targetIndex - 31;  // Shift amount is positive.
                prefix |= nextBlock;
                numRequested -= 32;
                targetIndex -= 32;

                if (numRequested > 0 && --blockIndex >= 0)
                {
                    // We know that targetIndex > 0; only 'double' allows us
                    // to get here, so numRequested is at most 53.  We also
                    // know that targetIndex < 32 because we started with 63
                    // and subtracted at least 32 from it.  Thus, the shift
                    // amount is positive.
                    nextBlock = bits[blockIndex];
                    nextBlock >>= 31 - targetIndex;
                    prefix |= nextBlock;
                }
            }

            return prefix;
        }
    };
}
