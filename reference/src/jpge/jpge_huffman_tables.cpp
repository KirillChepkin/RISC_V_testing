#include "jpge_huffman_tables.h"
#include <string.h>

namespace jpge {
enum { DC_LUM_CODES = 12, AC_LUM_CODES = 256, DC_CHROMA_CODES = 12, AC_CHROMA_CODES = 256, MAX_HUFF_SYMBOLS = 257, MAX_HUFF_CODESIZE = 32 };
template <class T> inline void clear_obj(T &obj) { memset(&obj, 0, sizeof(obj)); }
struct sym_freq { uint m_key, m_sym_index; };

static inline sym_freq* radix_sort_syms(uint num_syms, sym_freq* pSyms0, sym_freq* pSyms1)
{
  const uint cMaxPasses = 4;
  uint32 hist[256 * cMaxPasses]; clear_obj(hist);
  for (uint i = 0; i < num_syms; i++) { uint freq = pSyms0[i].m_key; hist[freq & 0xFF]++; hist[256 + ((freq >> 8) & 0xFF)]++; hist[256*2 + ((freq >> 16) & 0xFF)]++; hist[256*3 + ((freq >> 24) & 0xFF)]++; }
  sym_freq* pCur_syms = pSyms0, *pNew_syms = pSyms1;
  uint total_passes = cMaxPasses; while ((total_passes > 1) && (num_syms == hist[(total_passes - 1) * 256])) total_passes--;
  for (uint pass_shift = 0, pass = 0; pass < total_passes; pass++, pass_shift += 8)
  {
    const uint32* pHist = &hist[pass << 8];
    uint offsets[256], cur_ofs = 0;
    for (uint i = 0; i < 256; i++) { offsets[i] = cur_ofs; cur_ofs += pHist[i]; }
    for (uint i = 0; i < num_syms; i++)
      pNew_syms[offsets[(pCur_syms[i].m_key >> pass_shift) & 0xFF]++] = pCur_syms[i];
    sym_freq* t = pCur_syms; pCur_syms = pNew_syms; pNew_syms = t;
  }
  return pCur_syms;
}

static void calculate_minimum_redundancy(sym_freq *A, int n)
{
  int root, leaf, next, avbl, used, dpth;
  if (n==0) return; else if (n==1) { A[0].m_key = 1; return; }
  A[0].m_key += A[1].m_key; root = 0; leaf = 2;
  for (next=1; next < n-1; next++)
  {
    if (leaf>=n || A[root].m_key<A[leaf].m_key) { A[next].m_key = A[root].m_key; A[root++].m_key = next; } else A[next].m_key = A[leaf++].m_key;
    if (leaf>=n || (root<next && A[root].m_key<A[leaf].m_key)) { A[next].m_key += A[root].m_key; A[root++].m_key = next; } else A[next].m_key += A[leaf++].m_key;
  }

  A[n-2].m_key = 0;
  for (next=n-3; next>=0; next--) A[next].m_key = A[A[next].m_key].m_key+1;
  avbl = 1; used = dpth = 0; root = n-2; next = n-1;
  while (avbl>0)
  {
    while (root>=0 && (int)A[root].m_key==dpth) { used++; root--; }
    while (avbl>used) { A[next--].m_key = dpth; avbl--; }
    avbl = 2*used; dpth++; used = 0;
  }
}

static void huffman_enforce_max_code_size(int *pNum_codes, int code_list_len, int max_code_size)
{
  if (code_list_len <= 1) return;

  for (int i = max_code_size + 1; i <= MAX_HUFF_CODESIZE; i++) pNum_codes[max_code_size] += pNum_codes[i];

  uint32 total = 0;
  for (int i = max_code_size; i > 0; i--)
    total += (((uint32)pNum_codes[i]) << (max_code_size - i));

  while (total != (1UL << max_code_size))
  {
    pNum_codes[max_code_size]--;
    for (int i = max_code_size - 1; i > 0; i--)
    {
      if (pNum_codes[i]) { pNum_codes[i]--; pNum_codes[i + 1] += 2; break; }
    }
    total--;
  }
}

void jpeg_encoder::optimize_huffman_table(int table_num, int table_len)
{
  sym_freq syms0[MAX_HUFF_SYMBOLS], syms1[MAX_HUFF_SYMBOLS];
  syms0[0].m_key = 1; syms0[0].m_sym_index = 0;  // dummy symbol, assures that no valid code contains all 1's
  int num_used_syms = 1;
  const uint32 *pSym_count = &m_huff_count[table_num][0];
  for (int i = 0; i < table_len; i++)
    if (pSym_count[i]) { syms0[num_used_syms].m_key = pSym_count[i]; syms0[num_used_syms++].m_sym_index = i + 1; }
  sym_freq* pSyms = radix_sort_syms(num_used_syms, syms0, syms1);
  calculate_minimum_redundancy(pSyms, num_used_syms);

  // Count the # of symbols of each code size.
  int num_codes[1 + MAX_HUFF_CODESIZE]; clear_obj(num_codes);
  for (int i = 0; i < num_used_syms; i++)
    num_codes[pSyms[i].m_key]++;

  const uint JPGE_CODE_SIZE_LIMIT = 16; // the maximum possible size of a JPEG Huffman code (valid range is [9,16] - 9 vs. 8 because of the dummy symbol)
  huffman_enforce_max_code_size(num_codes, num_used_syms, JPGE_CODE_SIZE_LIMIT);

  // Compute m_huff_bits array, which contains the # of symbols per code size.
  clear_obj(m_huff_bits[table_num]);
  for (int i = 1; i <= (int)JPGE_CODE_SIZE_LIMIT; i++)
    m_huff_bits[table_num][i] = static_cast<uint8>(num_codes[i]);

  // Remove the dummy symbol added above, which must be in largest bucket.
  for (int i = JPGE_CODE_SIZE_LIMIT; i >= 1; i--)
  {
    if (m_huff_bits[table_num][i]) { m_huff_bits[table_num][i]--; break; }
  }

  // Compute the m_huff_val array, which contains the symbol indices sorted by code size (smallest to largest).
  for (int i = num_used_syms - 1; i >= 1; i--)
    m_huff_val[table_num][num_used_syms - 1 - i] = static_cast<uint8>(pSyms[i].m_sym_index - 1);
}

void jpeg_encoder::compute_huffman_table(uint *codes, uint8 *code_sizes, uint8 *bits, uint8 *val)
{
  int i, l, last_p, si;
  uint8 huff_size[257];
  uint huff_code[257];
  uint code;

  int p = 0;
  for (l = 1; l <= 16; l++)
    for (i = 1; i <= bits[l]; i++)
      huff_size[p++] = (char)l;

  huff_size[p] = 0; last_p = p; // write sentinel

  code = 0; si = huff_size[0]; p = 0;

  while (huff_size[p])
  {
    while (huff_size[p] == si)
      huff_code[p++] = code++;
    code <<= 1;
    si++;
  }

  memset(codes, 0, sizeof(codes[0])*256);
  memset(code_sizes, 0, sizeof(code_sizes[0])*256);
  for (p = 0; p < last_p; p++)
  {
    codes[val[p]]      = huff_code[p];
    code_sizes[val[p]] = huff_size[p];
  }
}
} // namespace jpge
