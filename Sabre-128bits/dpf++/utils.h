std::array<block<__m256i> , 128>  ___transpose(const std::array<__m128i, 256>& input)
{
  
      std::array<block<__m256i> , 128>   inputT;
  
	  for(size_t i = 0; i < 128; ++i)
	  {
	    for(size_t j = 0; j < 256; ++j)
	    {
	      inputT[i].bits[j] = (block<__m128i>(input[j])).bits[i]; 
	    }
	  }
	 
	 return inputT;
}

std::array<block<__m256i> , 256>  ___transpose(const std::array<__m256i, 256> input)
{
  
      std::array<block<__m256i> , 256>   inputT;
  
	  for(size_t i = 0; i < 256; ++i)
	  {
	    for(size_t j = 0; j < 256; ++j)
	    {
	      inputT[i].bits[j] = (block<__m256i>(input[j])).bits[i]; 
	    }
	  }
	 
	 return inputT;
}


static inline void trans(uint8_t const * inp, uint8_t * out, size_t nrows, size_t ncols)
{
    #define INP(x,y) inp[(x)*ncols/8 + (y)/8]
    #define OUT(x,y) out[(y)*nrows/8 + (x)/8]

    for (size_t row = 0; row < nrows; row += sizeof(__m256i))
    {
      for (size_t col = 0; col < ncols; col += 8)
      {
        __m256i x = _mm256_setr_epi8(INP(row + 0, col), INP(row + 1, col), INP(row + 2, col), INP(row + 3, col),
        INP(row + 4, col), INP(row + 5, col), INP(row + 6, col), INP(row + 7, col),
        INP(row + 8, col), INP(row + 9, col), INP(row + 10, col), INP(row + 11, col),
        INP(row + 12, col), INP(row + 13, col), INP(row + 14, col), INP(row + 15, col),
        INP(row + 16, col), INP(row + 17, col), INP(row + 18, col), INP(row + 19, col),
        INP(row + 20, col), INP(row + 21, col), INP(row + 22, col), INP(row + 23, col),
        INP(row + 24, col), INP(row + 25, col), INP(row + 26, col), INP(row + 27, col),
        INP(row + 28, col), INP(row + 29, col), INP(row + 30, col), INP(row + 31, col));

        *(uint32_t*)&OUT(row, col+7)= _mm256_movemask_epi8(_mm256_slli_epi64(x, 0));
        *(uint32_t*)&OUT(row, col+6)= _mm256_movemask_epi8(_mm256_slli_epi64(x, 1));
        *(uint32_t*)&OUT(row, col+5)= _mm256_movemask_epi8(_mm256_slli_epi64(x, 2));
        *(uint32_t*)&OUT(row, col+4)= _mm256_movemask_epi8(_mm256_slli_epi64(x, 3));
        *(uint32_t*)&OUT(row, col+3)= _mm256_movemask_epi8(_mm256_slli_epi64(x, 4));
        *(uint32_t*)&OUT(row, col+2)= _mm256_movemask_epi8(_mm256_slli_epi64(x, 5));
        *(uint32_t*)&OUT(row, col+1)= _mm256_movemask_epi8(_mm256_slli_epi64(x, 6));
        *(uint32_t*)&OUT(row, col+0)= _mm256_movemask_epi8(_mm256_slli_epi64(x, 7));
      }
    }
}


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