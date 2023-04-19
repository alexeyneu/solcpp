#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <algorithm>

namespace solana {
static const int8_t b58digits_map[128] = {
  -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
  -1, 0, 1, 2, 3, 4, 5, 6,  7, 8,-1,-1,-1,-1,-1,-1,
  -1, 9,10,11,12,13,14,15, 16,-1,17,18,19,20,21,-1,
  22,23,24,25,26,27,28,29, 30,31,32,-1,-1,-1,-1,-1,
  -1,33,34,35,36,37,38,39, 40,41,42,43,-1,44,45,46,
  47,48,49,50,51,52,53,54, 55,56,57,-1,-1,-1,-1,-1,
};


inline bool b58tobin(void *bin /* out */ , size_t *binszp /* in - out */ , const char *b58 /* in */ , size_t b58sz /* in */ )
{
  size_t binsz = *binszp;
  const unsigned char *b58u = (const unsigned char *)b58;
  unsigned char *binu = (unsigned char *)bin;
  size_t outsz = (binsz + 3) / 4;
  uint32_t *out = new uint32_t[outsz]();
  uint64_t t;
  uint32_t c;
  size_t i, j;
  uint8_t bytesleft = binsz % 4;
  uint32_t zeromask = bytesleft == 0 ? 0 : (0xffffffff << (bytesleft * 8));
  unsigned int zerocount = 0;
  
  if (!b58sz)
    b58sz = strlen(b58);
  
  // Leading zeros, just count
  for (i = 0; i < b58sz && b58u[i] == '1'; i++)
    zerocount++;
  
  for ( ; i < b58sz; i++)
  {
    if (b58u[i] & 0x80)
    {
      delete[] out;

      // High-bit set on invalid digit
      return false;
    }
    if (b58digits_map[b58u[i]] == -1)
    {
      delete[] out;

      // Invalid base58 digit
      return false;
    }
    c = (unsigned short)b58digits_map[b58u[i]];
    for (j = outsz - 1; j + 1 > 0; j--)     
    {
      t = out[j] * 58ULL + c;
      c = (t  >> 32) & 0x3fULL;
      out[j] = t & 0xffffffffULL;
    }
    if (c)
    {
      delete[] out;

      // Output number too big (carry to the next int32)
      return false;
    }
    if (out[0] & zeromask)
    {
      // Output number too big (last int32 filled too far)
      delete[] out;
      return false;
    }
  }
  
  j = 0;
  switch (bytesleft) 
  {
    case 3:
      *(binu++) = *((uint8_t*)out + 2);
      /* Fall Through */ 
    case 2:
      *(binu++) = *((uint8_t*)out + 1);
      /* Fall Through */ 
    case 1:
      *(binu++) = *(uint8_t*)out;
      j++;
      break;
    default:
      break;
  }
  
  for (; j < outsz; j++)
  {
    std::reverse((char *)(out + j), (char *)(out + j) + 4);
    *(uint32_t *)binu =  out[j];
    binu = binu + 4;
  }
  delete[] out;
  //size of the result binary,modified that way that the number of leading zeroes in it replaced by the count of leading '1' symbols in given string.
  binu = (unsigned char *)bin;
  for (i = 0; i < binsz; i++)
  {
    if (binu[i])
      break;
    (*binszp)--;
  }
  *binszp = *binszp + zerocount;
  
  return true;
}

static const char b58digits_ordered[59] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

inline bool b58enc(char* b58 /* out */ ,  size_t *b58sz /* in - out */ , const void *data /* in */ , size_t binsz /* in */ )
{
  const uint8_t *bin = (const uint8_t *)data;
  int carry;
  long i, j, high, zcount = 0;  
  while (zcount < binsz && !bin[zcount])
    zcount++;
  const unsigned int size = (binsz - zcount) * 138 / 100 + 1; //latter is a smth like a logarithm of 256 to base 58 , but not exactly.
  unsigned char *buf = new unsigned char[size]();

  high = size - 1;
  for (i = zcount; i < binsz; i++)
  {
    carry = bin[i]; j = size - 1;
    while( carry || j > high )
    {
      carry = carry + 256 * buf[j];
      buf[j--] = carry % 58;  //as you all know 'int fifty_cent() { int j = 0; return j-- ; }' has a zero value      
      carry /= 58;
    }
    high = j;
  }
  
  for (j = 0; j < size && !buf[j]; j++);
  
  if (*b58sz < zcount + size - j + 1)
  {
    *b58sz = zcount + size - j + 1;
    delete[] buf;
    return false;
  }

  if (zcount)
    memset(b58, '1', zcount);

  for (i = zcount; j < size; i++, j++)
    b58[i] = b58digits_ordered[buf[j]];

  delete[] buf;
  b58[i] = '\0';
  *b58sz = i + 1;
  return true;
}

inline std::pair<int, std::string> b58decode(const std::string &b58) {
  if (b58.size() > LONG_MAX >> 4) return std::pair(-7, std::string());
  size_t decodedSize = b58.size() << 3;;
  unsigned char *bc = new unsigned char[decodedSize]();
  bool ok = b58tobin((void *)bc, &decodedSize, b58.c_str(), b58.length());
  if (ok) {
    ok = b58tobin((void *)bc, &decodedSize, b58.c_str(), b58.length());
    auto pr = std::pair(1, std::string((char *)bc, decodedSize));
    delete[] bc;
    return pr;
  } 
  else
  { 
    delete[] bc; 
    return std::pair(-8, std::string());
  }
}

template <typename T>
inline std::pair<int, std::string> b58encode(const T &chvect) {
  if (chvect.size() > LONG_MAX >> 4) return std::pair(-7, std::string());
  size_t b58Size = chvect.size() << 3;
  char *b58c = new char[b58Size];
  bool ok = b58enc(b58c, &b58Size, (void *)chvect.data(), chvect.size());
  if (ok) {
    auto pr = std::pair(1, std::string(b58c));
    delete[] b58c;
    return pr;
  } 
  else
  {
    delete[] b58c;
    return std::pair(-8, std::string());
  }
}
}  // namespace solana