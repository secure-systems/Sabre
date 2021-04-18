/** Sabre, an anonymous bulletin board with speedier ripostes
 *  Copyright (C) 2020  Sabre authors
 *
 *  @file    transposition.h
 *  @brief   
 *
 *  @author  Ryan Henry        <ryan.henry@ucalgary.ca>
 *  @author  Adithya Vadapalli <avadapal@iu.edu>
 *  @author  Kyle Storrier     <kyle.storrier@ucalgary.ca>
 *
 *  @license GNU Public License (version 2); see LICENSE for full license text
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **/

#ifndef LOWMC_TRANSPOSITION_H__
#define LOWMC_TRANSPOSITION_H__

namespace lowmc
{

namespace dpf
{

inline auto get_bytes_from_bits(const __m256i & t, int which)
{
    static const unsigned char mask1a[32] = {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01,
        0x02, 0x02, 0x02, 0x02,
        0x02, 0x02, 0x02, 0x02,
        0x03, 0x03, 0x03, 0x03,
        0x03, 0x03, 0x03, 0x03
    };

    static const unsigned char mask2a[32] = {
        0x01, 0x02, 0x04, 0x08,
        0x10, 0x20, 0x40, 0x80,
        0x01, 0x02, 0x04, 0x08,
        0x10, 0x20, 0x40, 0x80,
        0x01, 0x02, 0x04, 0x08,
        0x10, 0x20, 0x40, 0x80,
        0x01, 0x02, 0x04, 0x08,
        0x10, 0x20, 0x40, 0x80,
    };

    __m256i mask2 = _mm256_loadu_si256((__m256i*)mask2a);
    __m256i mask1 = _mm256_loadu_si256((__m256i*)mask1a);

    auto y = _mm256_permutevar8x32_epi32(t, _mm256_set1_epi32(which));
    auto z = _mm256_shuffle_epi8(y, mask1);
    return _mm256_and_si256(z, mask2);
}

template <int imm8 = 8>
inline static __m256i _mm256_gatherbytes_epi8(const __m256i * base_addr)
{
    return _mm256_setr_epi8(
        _mm256_extract_epi8(base_addr[ 0], imm8),
        _mm256_extract_epi8(base_addr[ 1], imm8),
        _mm256_extract_epi8(base_addr[ 2], imm8),
        _mm256_extract_epi8(base_addr[ 3], imm8),
        _mm256_extract_epi8(base_addr[ 4], imm8),
        _mm256_extract_epi8(base_addr[ 5], imm8),
        _mm256_extract_epi8(base_addr[ 6], imm8),
        _mm256_extract_epi8(base_addr[ 7], imm8),
        _mm256_extract_epi8(base_addr[ 8], imm8),
        _mm256_extract_epi8(base_addr[ 9], imm8),
        _mm256_extract_epi8(base_addr[10], imm8),
        _mm256_extract_epi8(base_addr[11], imm8),
        _mm256_extract_epi8(base_addr[12], imm8),
        _mm256_extract_epi8(base_addr[13], imm8),
        _mm256_extract_epi8(base_addr[14], imm8),
        _mm256_extract_epi8(base_addr[15], imm8),
        _mm256_extract_epi8(base_addr[16], imm8),
        _mm256_extract_epi8(base_addr[17], imm8),
        _mm256_extract_epi8(base_addr[18], imm8),
        _mm256_extract_epi8(base_addr[19], imm8),
        _mm256_extract_epi8(base_addr[20], imm8),
        _mm256_extract_epi8(base_addr[21], imm8),
        _mm256_extract_epi8(base_addr[22], imm8),
        _mm256_extract_epi8(base_addr[23], imm8),
        _mm256_extract_epi8(base_addr[24], imm8),
        _mm256_extract_epi8(base_addr[25], imm8),
        _mm256_extract_epi8(base_addr[26], imm8),
        _mm256_extract_epi8(base_addr[27], imm8),
        _mm256_extract_epi8(base_addr[28], imm8),
        _mm256_extract_epi8(base_addr[29], imm8),
        _mm256_extract_epi8(base_addr[30], imm8),
        _mm256_extract_epi8(base_addr[31], imm8)
    );
}

/// transpose a 32-by-8 matrix of bits
/// the result is an std::tuple consisting of eight 32-bit rows
inline static auto transpose32x8(const __m256i & x)
{
	return std::make_tuple(_mm256_movemask_epi8(x),
		    _mm256_movemask_epi8(_mm256_slli_epi64(x, 1)),
		    _mm256_movemask_epi8(_mm256_slli_epi64(x, 2)),
		    _mm256_movemask_epi8(_mm256_slli_epi64(x, 3)),
		    _mm256_movemask_epi8(_mm256_slli_epi64(x, 4)),
		    _mm256_movemask_epi8(_mm256_slli_epi64(x, 5)),
		    _mm256_movemask_epi8(_mm256_slli_epi64(x, 6)),
		    _mm256_movemask_epi8(_mm256_slli_epi64(x, 7))
		);
}

/// construct a 32-by-8 matrix of bits from a 1-byte column of 32 consecutive rows
template<size_t slices, size_t byte>
inline static auto extract_bytes(const std::array<__m128i, slices> & x, size_t chunk)
{
	return _mm256_set_epi8(
	    _mm_extract_epi8(x[chunk+0 ], byte), _mm_extract_epi8(x[chunk+1 ], byte),
	    _mm_extract_epi8(x[chunk+2 ], byte), _mm_extract_epi8(x[chunk+3 ], byte),
	    _mm_extract_epi8(x[chunk+4 ], byte), _mm_extract_epi8(x[chunk+5 ], byte),
	    _mm_extract_epi8(x[chunk+6 ], byte), _mm_extract_epi8(x[chunk+7 ], byte),
	    _mm_extract_epi8(x[chunk+8 ], byte), _mm_extract_epi8(x[chunk+9 ], byte),
	    _mm_extract_epi8(x[chunk+10], byte), _mm_extract_epi8(x[chunk+11], byte),
	    _mm_extract_epi8(x[chunk+12], byte), _mm_extract_epi8(x[chunk+13], byte),
	    _mm_extract_epi8(x[chunk+14], byte), _mm_extract_epi8(x[chunk+15], byte),
	    _mm_extract_epi8(x[chunk+16], byte), _mm_extract_epi8(x[chunk+17], byte),
	    _mm_extract_epi8(x[chunk+18], byte), _mm_extract_epi8(x[chunk+19], byte),
	    _mm_extract_epi8(x[chunk+20], byte), _mm_extract_epi8(x[chunk+21], byte),
	    _mm_extract_epi8(x[chunk+22], byte), _mm_extract_epi8(x[chunk+23], byte),
	    _mm_extract_epi8(x[chunk+24], byte), _mm_extract_epi8(x[chunk+25], byte),
	    _mm_extract_epi8(x[chunk+26], byte), _mm_extract_epi8(x[chunk+27], byte),
	    _mm_extract_epi8(x[chunk+28], byte), _mm_extract_epi8(x[chunk+29], byte),
	    _mm_extract_epi8(x[chunk+30], byte), _mm_extract_epi8(x[chunk+31], byte)
	);
}

/// transpose a 1-byte column from 32 consecutive rows, and place the result in their final location
template <size_t slices, size_t byte>
inline static void transpose_chunk(const std::array<__m128i, slices> & x, std::array<uint32_t[4], slices> &T, size_t chunk)
{
	auto tmp = extract_bytes<15-byte>(x, chunk * sizeof(__m256i));
	std::tie(T[8*byte + 0][3 - chunk],
		     T[8*byte + 1][3 - chunk],
		     T[8*byte + 2][3 - chunk],
		     T[8*byte + 3][3 - chunk],
		     T[8*byte + 4][3 - chunk],
		     T[8*byte + 5][3 - chunk],
		     T[8*byte + 6][3 - chunk],
		     T[8*byte + 7][3 - chunk]) = transpose32x8(tmp);
}

template<typename block_t>
inline static std::array<block_t, 1> transpose(const block_t & val) { return { val }; }

template<size_t slices>
inline static auto transpose(const std::array<__m128i, slices> & arr)
{
	using out_type = std::conditional_t<slices == 128, __m128i, uint64_t>;
	std::array<out_type, 128> transpose;
	auto T = *reinterpret_cast<std::array<uint32_t[sizeof(out_type)/sizeof(uint32_t)], slices>*>(&transpose);
	for (size_t chunk = 0; chunk < slices/sizeof(__m256i); ++chunk)
	{
		transpose_chunk<0 >(arr, T, chunk);
		transpose_chunk<1 >(arr, T, chunk);
		transpose_chunk<2 >(arr, T, chunk);
		transpose_chunk<3 >(arr, T, chunk);
		transpose_chunk<4 >(arr, T, chunk);
		transpose_chunk<5 >(arr, T, chunk);
		transpose_chunk<6 >(arr, T, chunk);
		transpose_chunk<7 >(arr, T, chunk);
		transpose_chunk<8 >(arr, T, chunk);
		transpose_chunk<9 >(arr, T, chunk);
		transpose_chunk<10>(arr, T, chunk);
		transpose_chunk<11>(arr, T, chunk);
		transpose_chunk<12>(arr, T, chunk);
		transpose_chunk<13>(arr, T, chunk);
		transpose_chunk<14>(arr, T, chunk);
		transpose_chunk<15>(arr, T, chunk);
	}
	return std::move(transpose);
}


} // namespace dpf

} // namesapce lowmc

#endif // LOWMC_TRANSPOSITION_H__
