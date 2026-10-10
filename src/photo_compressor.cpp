#include "jpg/jpge.h"

#include <iostream>

extern "C" {
    typedef unsigned char stbi_uc;

    stbi_uc* stbi_load(
        const char* filename,
        int* x,
        int* y,
        int* comp,
        int req_comp
    );

    const char* stbi_failure_reason(void);

    void stbi_image_free(void* retval_from_stbi_load);
}

int main() {
    std::string filename;

    std::cout << "Enter input filename (from images/uncompressed/): ";
    std::getline(std::cin, filename);

    if (filename.empty()
            || filename.find('/') != std::string::npos
            || filename.find('\\') != std::string::npos) {
        std::cerr << "Invalid filename. Enter only the filename, "
                     "without a directory path.\n";
        return 1;
    }

    const std::string input_path = "images/uncompressed/" + filename;

    int width = 0;
    int height = 0;
    int original_components = 0;

    stbi_uc* image = stbi_load(
        input_path.c_str(),
        &width,
        &height,
        &original_components,
        3
    );

    if (image == nullptr) {
        std::cerr << "Failed to load image: "
                  << stbi_failure_reason() << '\n';
        return 1;
    }

    std::cout << "Image loaded successfully!\n";
    std::cout << "Width: " << width << '\n';
    std::cout << "Height: " << height << '\n';

    int quality = 0;

    std::cout << "Enter JPEG quality (1-100): ";

    if (!(std::cin >> quality) || quality < 1 || quality > 100) {
        std::cerr << "Invalid quality. Enter an integer from 1 to 100.\n";
        stbi_image_free(image);
        return 1;
    }

    std::string base_name = filename;
    const std::string::size_type dot = base_name.find_last_of('.');

    if (dot != std::string::npos) {
        base_name.erase(dot);
    }

    const std::string output_path =
        "images/compressed/" + base_name + "_q"
        + std::to_string(quality) + ".jpeg";

    jpge::params params;
    params.m_quality = quality;

    const bool success = jpge::compress_image_to_jpeg_file(
        output_path.c_str(),
        width,
        height,
        3,
        image,
        params
    );

    stbi_image_free(image);

    if (!success) {
        std::cerr << "Failed to compress image.\n";
        return 1;
    }

    std::cout << "Image compressed successfully!\n";
    std::cout << "Output: " << output_path << '\n';

    return 0;
}