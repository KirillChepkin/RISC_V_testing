#include "jpgd.h"

namespace jpgd {
// Clamps a value between 0-255.
inline uint8 jpeg_decoder::clamp(int i)
{
  if (static_cast<uint>(i) > 255)
    i = (((~i) >> 31) & 0xFF);

  return static_cast<uint8>(i);
}

// YCbCr H1V1 (1x1:1:1, 3 m_blocks per MCU) to RGB
void jpeg_decoder::H1V1Convert()
{
  int row = m_max_mcu_y_size - m_mcu_lines_left;
  uint8 *d = m_pScan_line_0;
  uint8 *s = m_pSample_buf + row * 8;

  for (int i = m_max_mcus_per_row; i > 0; i--)
  {
    for (int j = 0; j < 8; j++)
    {
      int y = s[j];
      int cb = s[64+j];
      int cr = s[128+j];

      d[0] = clamp(y + m_crr[cr]);
      d[1] = clamp(y + ((m_crg[cr] + m_cbg[cb]) >> 16));
      d[2] = clamp(y + m_cbb[cb]);
      d[3] = 255;

      d += 4;
    }

    s += 64*3;
  }
}

// YCbCr H2V1 (2x1:1:1, 4 m_blocks per MCU) to RGB
void jpeg_decoder::H2V1Convert()
{
  int row = m_max_mcu_y_size - m_mcu_lines_left;
  uint8 *d0 = m_pScan_line_0;
  uint8 *y = m_pSample_buf + row * 8;
  uint8 *c = m_pSample_buf + 2*64 + row * 8;

  for (int i = m_max_mcus_per_row; i > 0; i--)
  {
    for (int l = 0; l < 2; l++)
    {
      for (int j = 0; j < 4; j++)
      {
        int cb = c[0];
        int cr = c[64];

        int rc = m_crr[cr];
        int gc = ((m_crg[cr] + m_cbg[cb]) >> 16);
        int bc = m_cbb[cb];

        int yy = y[j<<1];
        d0[0] = clamp(yy+rc);
        d0[1] = clamp(yy+gc);
        d0[2] = clamp(yy+bc);
        d0[3] = 255;

        yy = y[(j<<1)+1];
        d0[4] = clamp(yy+rc);
        d0[5] = clamp(yy+gc);
        d0[6] = clamp(yy+bc);
        d0[7] = 255;

        d0 += 8;

        c++;
      }
      y += 64;
    }

    y += 64*4 - 64*2;
    c += 64*4 - 8;
  }
}

// YCbCr H2V1 (1x2:1:1, 4 m_blocks per MCU) to RGB
void jpeg_decoder::H1V2Convert()
{
  int row = m_max_mcu_y_size - m_mcu_lines_left;
  uint8 *d0 = m_pScan_line_0;
  uint8 *d1 = m_pScan_line_1;
  uint8 *y;
  uint8 *c;

  if (row < 8)
    y = m_pSample_buf + row * 8;
  else
    y = m_pSample_buf + 64*1 + (row & 7) * 8;

  c = m_pSample_buf + 64*2 + (row >> 1) * 8;

  for (int i = m_max_mcus_per_row; i > 0; i--)
  {
    for (int j = 0; j < 8; j++)
    {
      int cb = c[0+j];
      int cr = c[64+j];

      int rc = m_crr[cr];
      int gc = ((m_crg[cr] + m_cbg[cb]) >> 16);
      int bc = m_cbb[cb];

      int yy = y[j];
      d0[0] = clamp(yy+rc);
      d0[1] = clamp(yy+gc);
      d0[2] = clamp(yy+bc);
      d0[3] = 255;

      yy = y[8+j];
      d1[0] = clamp(yy+rc);
      d1[1] = clamp(yy+gc);
      d1[2] = clamp(yy+bc);
      d1[3] = 255;

      d0 += 4;
      d1 += 4;
    }

    y += 64*4;
    c += 64*4;
  }
}

// YCbCr H2V2 (2x2:1:1, 6 m_blocks per MCU) to RGB
void jpeg_decoder::H2V2Convert()
{
	int row = m_max_mcu_y_size - m_mcu_lines_left;
	uint8 *d0 = m_pScan_line_0;
	uint8 *d1 = m_pScan_line_1;
	uint8 *y;
	uint8 *c;

	if (row < 8)
		y = m_pSample_buf + row * 8;
	else
		y = m_pSample_buf + 64*2 + (row & 7) * 8;

	c = m_pSample_buf + 64*4 + (row >> 1) * 8;

	for (int i = m_max_mcus_per_row; i > 0; i--)
	{
		for (int l = 0; l < 2; l++)
		{
			for (int j = 0; j < 8; j += 2)
			{
				int cb = c[0];
				int cr = c[64];

				int rc = m_crr[cr];
				int gc = ((m_crg[cr] + m_cbg[cb]) >> 16);
				int bc = m_cbb[cb];

				int yy = y[j];
				d0[0] = clamp(yy+rc);
				d0[1] = clamp(yy+gc);
				d0[2] = clamp(yy+bc);
				d0[3] = 255;

				yy = y[j+1];
				d0[4] = clamp(yy+rc);
				d0[5] = clamp(yy+gc);
				d0[6] = clamp(yy+bc);
				d0[7] = 255;

				yy = y[j+8];
				d1[0] = clamp(yy+rc);
				d1[1] = clamp(yy+gc);
				d1[2] = clamp(yy+bc);
				d1[3] = 255;

				yy = y[j+8+1];
				d1[4] = clamp(yy+rc);
				d1[5] = clamp(yy+gc);
				d1[6] = clamp(yy+bc);
				d1[7] = 255;

				d0 += 8;
				d1 += 8;

				c++;
			}
			y += 64;
		}

		y += 64*6 - 64*2;
		c += 64*6 - 8;
	}
}

// Y (1 block per MCU) to 8-bit grayscale
void jpeg_decoder::gray_convert()
{
  int row = m_max_mcu_y_size - m_mcu_lines_left;
  uint8 *d = m_pScan_line_0;
  uint8 *s = m_pSample_buf + row * 8;

  for (int i = m_max_mcus_per_row; i > 0; i--)
  {
    *(uint *)d = *(uint *)s;
    *(uint *)(&d[4]) = *(uint *)(&s[4]);

    s += 64;
    d += 8;
  }
}

void jpeg_decoder::expanded_convert()
{
  int row = m_max_mcu_y_size - m_mcu_lines_left;

  uint8* Py = m_pSample_buf + (row / 8) * 64 * m_comp_h_samp[0] + (row & 7) * 8;

  uint8* d = m_pScan_line_0;

  for (int i = m_max_mcus_per_row; i > 0; i--)
  {
    for (int k = 0; k < m_max_mcu_x_size; k += 8)
    {
      const int Y_ofs = k * 8;
      const int Cb_ofs = Y_ofs + 64 * m_expanded_blocks_per_component;
      const int Cr_ofs = Y_ofs + 64 * m_expanded_blocks_per_component * 2;
      for (int j = 0; j < 8; j++)
      {
        int y = Py[Y_ofs + j];
        int cb = Py[Cb_ofs + j];
        int cr = Py[Cr_ofs + j];

        d[0] = clamp(y + m_crr[cr]);
        d[1] = clamp(y + ((m_crg[cr] + m_cbg[cb]) >> 16));
        d[2] = clamp(y + m_cbb[cb]);
        d[3] = 255;

        d += 4;
      }
    }

    Py += 64 * m_expanded_blocks_per_mcu;
  }
}
}