#include "../jpg/jpgd/jpgd.h"

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

    int stbi_write_tga(char const *filename, int x, int y, int comp, void *data);
}

int main() {
    std::string filename;

    std::cout << "Enter input filename (from images/compressed/): ";
    std::getline(std::cin, filename);

    if (filename.empty()
            || filename.find('/') != std::string::npos
            || filename.find('\\') != std::string::npos) {
        std::cerr << "Invalid filename. Enter only the filename, "
                     "without a directory path.\n";
        return 1;
    }

    const std::string input_path = "images/compressed/" + filename;

    int width = 0;
    int height = 0;
    int original_components = 0;
    int req_comp = 3;

    stbi_uc* image = stbi_load(
        input_path.c_str(),
        &width,
        &height,
        &original_components,
        req_comp
    );

    if (image == nullptr) {
        std::cerr << "Failed to load image: "
                  << stbi_failure_reason() << '\n';
        return 1;
    }

    std::cout << "Compressed image loaded successfully!\n";
    std::cout << "Width: " << width << '\n';
    std::cout << "Height: " << height << '\n';


    std::string base_name = filename;
    const std::string::size_type dot = base_name.find_last_of('.');

    if (dot != std::string::npos) {
        base_name.erase(dot);
    }

    const std::string output_path =
        "images/decompressed/" + base_name + ".tga";

    jpgd::uint8 *image_data = jpgd::decompress_jpeg_image_from_file(input_path.c_str(), &width, &height, &original_components, req_comp);

    int res = stbi_write_tga(output_path.c_str(), width, height, req_comp, image_data);

    if (!res) {
        std::cerr << "Failed to decompress image.\n";
        return 1;
    }

    std::cout << "Image decompressed successfully!\n";
    std::cout << "Output: " << output_path << '\n';

    return 0;
}