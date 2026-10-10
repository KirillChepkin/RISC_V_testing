#include "jpg/jpge.h"

#include <algorithm>
#include <cmath>
#include <vector>
#include <fstream>

int main()
{
    const int width = 800;
    const int height = 600;
    const int channels = 3;

    std::vector<unsigned char> image(width * height * channels);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            const float nx = static_cast<float>(x) / width;
            const float ny = static_cast<float>(y) / height;

            float r, g, b;

            if (ny < 0.62f)
            {
                // Небо: переход от синего к оранжевому.
                float t = ny / 0.62f;

                r = 20 + 245 * t;
                g = 35 + 105 * t;
                b = 90 + 20 * (1 - t);

                // Солнце.
                float dx = nx - 0.72f;
                float dy = ny - 0.43f;
                float d = std::sqrt(dx * dx + dy * dy);

                if (d < 0.105f)
                {
                    r = 255;
                    g = 210 + 40 * (1 - d / 0.105f);
                    b = 100 + 100 * (1 - d / 0.105f);
                }
            }
            else
            {
                // Озеро: отражение заката.
                float t = (ny - 0.62f) / 0.38f;

                r = 25 + 95 * (1 - t);
                g = 35 + 35 * (1 - t);
                b = 65 + 65 * (1 - t);

                // Световая дорожка от солнца.
                float center = 0.72f;
                float spread = 0.025f + 0.12f * t;

                if (std::abs(nx - center) < spread
                    && std::fmod(y, 12) < 5)
                {
                    r = 255;
                    g = 165;
                    b = 75;
                }
            }

            // Два горных хребта.
            float ridge1 = 0.48f
                + 0.07f * std::sin(nx * 18.0f)
                + 0.035f * std::sin(nx * 39.0f);

            float ridge2 = 0.57f
                + 0.045f * std::sin(nx * 23.0f + 1.0f)
                + 0.025f * std::sin(nx * 47.0f);

            if (ny > ridge1)
            {
                r = 38;
                g = 35;
                b = 68;
            }

            if (ny > ridge2)
            {
                r = 19;
                g = 28;
                b = 48;
            }

            int i = (y * width + x) * channels;

            image[i]     = static_cast<unsigned char>(
                std::clamp(r, 0.0f, 255.0f));
            image[i + 1] = static_cast<unsigned char>(
                std::clamp(g, 0.0f, 255.0f));
            image[i + 2] = static_cast<unsigned char>(
                std::clamp(b, 0.0f, 255.0f));
        }
    }
    jpge::params params;
    params.m_quality = 5;
    jpge::compress_image_to_jpeg_file(
        "output/test_q5.jpeg",
        width,
        height,
        channels,
        image.data(),
        params
    );
    return 0;
}
