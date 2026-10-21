#include "jpge_marker_writer.h"
#include <string.h>

namespace jpge {
enum { M_SOF0 = 0xC0, M_DHT = 0xC4, M_SOI = 0xD8, M_EOI = 0xD9, M_SOS = 0xDA, M_DQT = 0xDB, M_APP0 = 0xE0 };

void jpeg_encoder::emit_byte(uint8 i)
{
  m_all_stream_writes_succeeded = m_all_stream_writes_succeeded && m_pStream->put_obj(i);
}

void jpeg_encoder::emit_word(uint i)
{
  emit_byte(uint8(i >> 8)); emit_byte(uint8(i & 0xFF));
}

void jpeg_encoder::emit_marker(int marker)
{
  emit_byte(uint8(0xFF)); emit_byte(uint8(marker));
}

void jpeg_encoder::emit_jfif_app0()
{
  emit_marker(M_APP0);
  emit_word(2 + 4 + 1 + 2 + 1 + 2 + 2 + 1 + 1);
  emit_byte(0x4A); emit_byte(0x46); emit_byte(0x49); emit_byte(0x46); /* Identifier: ASCII "JFIF" */
  emit_byte(0);
  emit_byte(1);      /* Major version */
  emit_byte(1);      /* Minor version */
  emit_byte(0);      /* Density unit */
  emit_word(1);
  emit_word(1);
  emit_byte(0);      /* No thumbnail image */
  emit_byte(0);
}

void jpeg_encoder::emit_dqt()
{
  for (int i = 0; i < ((m_num_components == 3) ? 2 : 1); i++)
  {
    emit_marker(M_DQT);
    emit_word(64 + 1 + 2);
    emit_byte(static_cast<uint8>(i));
    for (int j = 0; j < 64; j++)
      emit_byte(static_cast<uint8>(m_quantization_tables[i][j]));
  }
}

void jpeg_encoder::emit_sof()
{
  emit_marker(M_SOF0);                           /* baseline */
  emit_word(3 * m_num_components + 2 + 5 + 1);
  emit_byte(8);                                  /* precision */
  emit_word(m_image_y);
  emit_word(m_image_x);
  emit_byte(m_num_components);
  for (int i = 0; i < m_num_components; i++)
  {
    emit_byte(static_cast<uint8>(i + 1));                                   /* component ID     */
    emit_byte((m_comp_h_samp[i] << 4) + m_comp_v_samp[i]);  /* h and v sampling */
    emit_byte(i > 0);                                   /* quant. table num */
  }
}

void jpeg_encoder::emit_dht(uint8 *bits, uint8 *val, int index, bool ac_flag)
{
  emit_marker(M_DHT);

  int length = 0;
  for (int i = 1; i <= 16; i++)
    length += bits[i];

  emit_word(length + 2 + 1 + 16);
  emit_byte(static_cast<uint8>(index + (ac_flag << 4)));

  for (int i = 1; i <= 16; i++)
    emit_byte(bits[i]);

  for (int i = 0; i < length; i++)
    emit_byte(val[i]);
}

void jpeg_encoder::emit_dhts()
{
  emit_dht(m_huff_bits[0+0], m_huff_val[0+0], 0, false);
  emit_dht(m_huff_bits[2+0], m_huff_val[2+0], 0, true);
  if (m_num_components == 3)
  {
    emit_dht(m_huff_bits[0+1], m_huff_val[0+1], 1, false);
    emit_dht(m_huff_bits[2+1], m_huff_val[2+1], 1, true);
  }
}

void jpeg_encoder::emit_sos()
{
  emit_marker(M_SOS);
  emit_word(2 * m_num_components + 2 + 1 + 3);
  emit_byte(m_num_components);
  for (int i = 0; i < m_num_components; i++)
  {
    emit_byte(static_cast<uint8>(i + 1));
    if (i == 0)
      emit_byte((0 << 4) + 0);
    else
      emit_byte((1 << 4) + 1);
  }
  emit_byte(0);     /* spectral selection */
  emit_byte(63);
  emit_byte(0);
}

void jpeg_encoder::emit_markers()
{
  emit_marker(M_SOI);
  emit_jfif_app0();
  emit_dqt();
  emit_sof();
  emit_dhts();
  emit_sos();
}

void jpeg_encoder::flush_output_buffer()
{
  if (m_out_buf_left != JPGE_OUT_BUF_SIZE)
    m_all_stream_writes_succeeded = m_all_stream_writes_succeeded && m_pStream->put_buf(m_out_buf, JPGE_OUT_BUF_SIZE - m_out_buf_left);
  m_pOut_buf = m_out_buf;
  m_out_buf_left = JPGE_OUT_BUF_SIZE;
}

void jpeg_encoder::put_bits(uint bits, uint len)
{
  m_bit_buffer |= ((uint32)bits << (24 - (m_bits_in += len)));
  while (m_bits_in >= 8)
  {
    uint8 c;
    #define JPGE_PUT_BYTE(c) { *m_pOut_buf++ = (c); if (--m_out_buf_left == 0) flush_output_buffer(); }
    JPGE_PUT_BYTE(c = (uint8)((m_bit_buffer >> 16) & 0xFF));
    if (c == 0xFF) JPGE_PUT_BYTE(0);
    m_bit_buffer <<= 8;
    m_bits_in -= 8;
  }
}

bool jpeg_encoder::terminate_pass_two()
{
  put_bits(0x7F, 7);
  flush_output_buffer();
  emit_marker(M_EOI);
  m_pass_num++; // purposely bump up m_pass_num, for debugging
  return true;
}
} // namespace jpge
