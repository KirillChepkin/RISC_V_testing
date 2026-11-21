struct params {
    inline params() : m_quality(85), m_subsampling(H2V2), m_no_chroma_discrim_flag(false), m_two_pass_flag(false) { }

    inline bool check() const
    {
      if ((m_quality < 1) || (m_quality > 100)) return false;
      if ((uint)m_subsampling > (uint)H2V2) return false;
      return true;
    }

    // Quality: 1-100, higher is better. Typical values are around 50-95.
    int m_quality;

    // m_subsampling:
    // 0 = Y (grayscale) only
    // 1 = YCbCr, no subsampling (H1V1, YCbCr 1x1x1, 3 blocks per MCU)
    // 2 = YCbCr, H2V1 subsampling (YCbCr 2x1x1, 4 blocks per MCU)
    // 3 = YCbCr, H2V2 subsampling (YCbCr 4x1x1, 6 blocks per MCU-- very common)
    subsampling_t m_subsampling;

    // Disables CbCr discrimination - only intended for testing.
    // If true, the Y quantization table is also used for the CbCr channels.
    bool m_no_chroma_discrim_flag;

    bool m_two_pass_flag;
};