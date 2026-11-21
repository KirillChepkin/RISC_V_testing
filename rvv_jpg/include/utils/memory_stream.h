#include <cstdio>
#include <output_stream.h>

class memory_stream : public output_stream
{
   memory_stream(const memory_stream &);
   memory_stream &operator= (const memory_stream &);

   uint8 *m_pBuf;
   uint m_buf_size, m_buf_ofs;

public:
   memory_stream(void *pBuf, uint buf_size) : m_pBuf(static_cast<uint8*>(pBuf)), m_buf_size(buf_size), m_buf_ofs(0) { }

   virtual ~memory_stream() { }

   virtual bool put_buf(const void* pBuf, int len)
   {
      uint buf_remaining = m_buf_size - m_buf_ofs;
      if ((uint)len > buf_remaining)
         return false;
      memcpy(m_pBuf + m_buf_ofs, pBuf, len);
      m_buf_ofs += len;
      return true;
   }

   uint get_size() const
   {
      return m_buf_ofs;
   }
};