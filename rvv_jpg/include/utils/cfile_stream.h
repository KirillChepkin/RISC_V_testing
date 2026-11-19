#include <cstdio>
#include <output_stream.h>

class cfile_stream : public output_stream
{
   cfile_stream(const cfile_stream &);
   cfile_stream &operator= (const cfile_stream &);

   FILE* m_pFile;
   bool m_bStatus;

public:
   cfile_stream() : m_pFile(NULL), m_bStatus(false) { }

   virtual ~cfile_stream()
   {
      close();
   }

   bool open(const char *pFilename)
   {
      close();
      m_pFile = fopen(pFilename, "wb");
      m_bStatus = (m_pFile != NULL);
      return m_bStatus;
   }

   bool close()
   {
      if (m_pFile)
      {
         if (fclose(m_pFile) == EOF)
         {
            m_bStatus = false;
         }
         m_pFile = NULL;
      }
      return m_bStatus;
   }

   virtual bool put_buf(const void* pBuf, int len)
   {
      m_bStatus = m_bStatus && (fwrite(pBuf, len, 1, m_pFile) == 1);
      return m_bStatus;
   }

   uint get_size() const
   {
      return m_pFile ? ftell(m_pFile) : 0;
   }
};