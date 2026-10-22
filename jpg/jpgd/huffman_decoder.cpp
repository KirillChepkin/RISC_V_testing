#include "jpgd.h"
#include "jpgd_scripts.h"
#include <string.h>

namespace jpgd {

// Decodes a Huffman encoded symbol.
int jpeg_decoder::huff_decode(huff_tables *pH)
{
  int symbol;

  // Check first 8-bits: do we have a complete symbol?
  if ((symbol = pH->look_up[m_bit_buf >> 24]) < 0)
  {
    // Decode more bits, use a tree traversal to find symbol.
    int ofs = 23;
    do
    {
      symbol = pH->tree[-(int)(symbol + ((m_bit_buf >> ofs) & 1))];
      ofs--;
    } while (symbol < 0);

    get_bits_no_markers(8 + (23 - ofs));
  }
  else
    get_bits_no_markers(pH->code_size[symbol]);

  return symbol;
}

// Decodes a Huffman encoded symbol.
int jpeg_decoder::huff_decode(huff_tables *pH, int& extra_bits)
{
  int symbol;

  // Check first 8-bits: do we have a complete symbol?
  if ((symbol = pH->look_up2[m_bit_buf >> 24]) < 0)
  {
    // Use a tree traversal to find symbol.
    int ofs = 23;
    do
    {
      symbol = pH->tree[-(int)(symbol + ((m_bit_buf >> ofs) & 1))];
      ofs--;
    } while (symbol < 0);

    get_bits_no_markers(8 + (23 - ofs));

    extra_bits = get_bits_no_markers(symbol & 0xF);
  }
  else
  {
    JPGD_ASSERT(((symbol >> 8) & 31) == pH->code_size[symbol & 255] + ((symbol & 0x8000) ? (symbol & 15) : 0));

    if (symbol & 0x8000)
    {
      get_bits_no_markers((symbol >> 8) & 31);
      extra_bits = symbol >> 16;
    }
    else
    {
      int code_size = (symbol >> 8) & 31;
      int num_extra_bits = symbol & 0xF;
      int bits = code_size + num_extra_bits;
      if (bits <= (m_bits_left + 16))
        extra_bits = get_bits_no_markers(bits) & ((1 << num_extra_bits) - 1);
      else
      {
        get_bits_no_markers(code_size);
        extra_bits = get_bits_no_markers(num_extra_bits);
      }
    }

    symbol &= 0xFF;
  }

  return symbol;
}


// Creates the tables needed for efficient Huffman decoding.
void jpeg_decoder::make_huff_table(int index, huff_tables *pH)
{
  int p, i, l, si;
  uint8 huffsize[257];
  uint huffcode[257];
  uint code;
  uint subtree;
  int code_size;
  int lastp;
  int nextfreeentry;
  int currententry;

  pH->ac_table = m_huff_ac[index] != 0;

  p = 0;

  for (l = 1; l <= 16; l++)
  {
    for (i = 1; i <= m_huff_num[index][l]; i++)
      huffsize[p++] = static_cast<uint8>(l);
  }

  huffsize[p] = 0;

  lastp = p;

  code = 0;
  si = huffsize[0];
  p = 0;

  while (huffsize[p])
  {
    while (huffsize[p] == si)
    {
      huffcode[p++] = code;
      code++;
    }

    code <<= 1;
    si++;
  }

  memset(pH->look_up, 0, sizeof(pH->look_up));
  memset(pH->look_up2, 0, sizeof(pH->look_up2));
  memset(pH->tree, 0, sizeof(pH->tree));
  memset(pH->code_size, 0, sizeof(pH->code_size));

  nextfreeentry = -1;

  p = 0;

  while (p < lastp)
  {
    i = m_huff_val[index][p];
    code = huffcode[p];
    code_size = huffsize[p];

    pH->code_size[i] = static_cast<uint8>(code_size);

    if (code_size <= 8)
    {
      code <<= (8 - code_size);

      for (l = 1 << (8 - code_size); l > 0; l--)
      {
        JPGD_ASSERT(i < 256);

        pH->look_up[code] = i;

        bool has_extrabits = false;
				int extra_bits = 0;
        int num_extra_bits = i & 15;

        int bits_to_fetch = code_size;
        if (num_extra_bits)
        {
          int total_codesize = code_size + num_extra_bits;
          if (total_codesize <= 8)
          {
            has_extrabits = true;
            extra_bits = ((1 << num_extra_bits) - 1) & (code >> (8 - total_codesize));
            JPGD_ASSERT(extra_bits <= 0x7FFF);
            bits_to_fetch += num_extra_bits;
          }
        }

        if (!has_extrabits)
          pH->look_up2[code] = i | (bits_to_fetch << 8);
        else
          pH->look_up2[code] = i | 0x8000 | (extra_bits << 16) | (bits_to_fetch << 8);

        code++;
      }
    }
    else
    {
      subtree = (code >> (code_size - 8)) & 0xFF;

      currententry = pH->look_up[subtree];

      if (currententry == 0)
      {
        pH->look_up[subtree] = currententry = nextfreeentry;
        pH->look_up2[subtree] = currententry = nextfreeentry;

        nextfreeentry -= 2;
      }

      code <<= (16 - (code_size - 8));

      for (l = code_size; l > 9; l--)
      {
        if ((code & 0x8000) == 0)
          currententry--;

        if (pH->tree[-currententry - 1] == 0)
        {
          pH->tree[-currententry - 1] = nextfreeentry;

          currententry = nextfreeentry;

          nextfreeentry -= 2;
        }
        else
          currententry = pH->tree[-currententry - 1];

        code <<= 1;
      }

      if ((code & 0x8000) == 0)
        currententry--;

      pH->tree[-currententry - 1] = i;
    }

    p++;
  }
}

// The following methods decode the various types of m_blocks encountered
// in progressively encoded images.
void jpeg_decoder::decode_block_dc_first(jpeg_decoder *pD, int component_id, int block_x, int block_y)
{
  int s, r;
  jpgd_block_t *p = pD->coeff_buf_getp(pD->m_dc_coeffs[component_id], block_x, block_y);

  if ((s = pD->huff_decode(pD->m_pHuff_tabs[pD->m_comp_dc_tab[component_id]])) != 0)
  {
    r = pD->get_bits_no_markers(s);
    s = JPGD_HUFF_EXTEND(r, s);
  }

  pD->m_last_dc_val[component_id] = (s += pD->m_last_dc_val[component_id]);

  p[0] = static_cast<jpgd_block_t>(s << pD->m_successive_low);
}

void jpeg_decoder::decode_block_dc_refine(jpeg_decoder *pD, int component_id, int block_x, int block_y)
{
  if (pD->get_bits_no_markers(1))
  {
    jpgd_block_t *p = pD->coeff_buf_getp(pD->m_dc_coeffs[component_id], block_x, block_y);

    p[0] |= (1 << pD->m_successive_low);
  }
}

void jpeg_decoder::decode_block_ac_first(jpeg_decoder *pD, int component_id, int block_x, int block_y)
{
  int k, s, r;

  if (pD->m_eob_run)
  {
    pD->m_eob_run--;
    return;
  }

  jpgd_block_t *p = pD->coeff_buf_getp(pD->m_ac_coeffs[component_id], block_x, block_y);

  for (k = pD->m_spectral_start; k <= pD->m_spectral_end; k++)
  {
    s = pD->huff_decode(pD->m_pHuff_tabs[pD->m_comp_ac_tab[component_id]]);

    r = s >> 4;
    s &= 15;

    if (s)
    {
      if ((k += r) > 63)
        pD->stop_decoding(JPGD_DECODE_ERROR);

      r = pD->get_bits_no_markers(s);
      s = JPGD_HUFF_EXTEND(r, s);

      p[g_ZAG[k]] = static_cast<jpgd_block_t>(s << pD->m_successive_low);
    }
    else
    {
      if (r == 15)
      {
        if ((k += 15) > 63)
          pD->stop_decoding(JPGD_DECODE_ERROR);
      }
      else
      {
        pD->m_eob_run = 1 << r;

        if (r)
          pD->m_eob_run += pD->get_bits_no_markers(r);

        pD->m_eob_run--;

        break;
      }
    }
  }
}

void jpeg_decoder::decode_block_ac_refine(jpeg_decoder *pD, int component_id, int block_x, int block_y)
{
  int s, k, r;
  int p1 = 1 << pD->m_successive_low;
  int m1 = (-1) << pD->m_successive_low;
  jpgd_block_t *p = pD->coeff_buf_getp(pD->m_ac_coeffs[component_id], block_x, block_y);
  
  JPGD_ASSERT(pD->m_spectral_end <= 63);
  
  k = pD->m_spectral_start;
  
  if (pD->m_eob_run == 0)
  {
    for ( ; k <= pD->m_spectral_end; k++)
    {
      s = pD->huff_decode(pD->m_pHuff_tabs[pD->m_comp_ac_tab[component_id]]);

      r = s >> 4;
      s &= 15;

      if (s)
      {
        if (s != 1)
          pD->stop_decoding(JPGD_DECODE_ERROR);

        if (pD->get_bits_no_markers(1))
          s = p1;
        else
          s = m1;
      }
      else
      {
        if (r != 15)
        {
          pD->m_eob_run = 1 << r;

          if (r)
            pD->m_eob_run += pD->get_bits_no_markers(r);

          break;
        }
      }

      do
      {
        jpgd_block_t *this_coef = p + g_ZAG[k & 63];

        if (*this_coef != 0)
        {
          if (pD->get_bits_no_markers(1))
          {
            if ((*this_coef & p1) == 0)
            {
              if (*this_coef >= 0)
                *this_coef = static_cast<jpgd_block_t>(*this_coef + p1);
              else
                *this_coef = static_cast<jpgd_block_t>(*this_coef + m1);
            }
          }
        }
        else
        {
          if (--r < 0)
            break;
        }

        k++;

      } while (k <= pD->m_spectral_end);

      if ((s) && (k < 64))
      {
        p[g_ZAG[k]] = static_cast<jpgd_block_t>(s);
      }
    }
  }

  if (pD->m_eob_run > 0)
  {
    for ( ; k <= pD->m_spectral_end; k++)
    {
      jpgd_block_t *this_coef = p + g_ZAG[k & 63]; // logical AND to shut up static code analysis

      if (*this_coef != 0)
      {
        if (pD->get_bits_no_markers(1))
        {
          if ((*this_coef & p1) == 0)
          {
            if (*this_coef >= 0)
              *this_coef = static_cast<jpgd_block_t>(*this_coef + p1);
            else
              *this_coef = static_cast<jpgd_block_t>(*this_coef + m1);
          }
        }
      }
    }

    pD->m_eob_run--;
  }
}

}