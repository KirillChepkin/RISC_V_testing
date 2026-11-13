#include <iostream>
#include <string>
#include <vector>
#include "jpge_internal_modules.h"

namespace {
int passed = 0;
int failed = 0;

uint8 clamp_ref(int value) {
    if (value < 0) return 0;
    if (value > 255) return 255;
    return static_cast<uint8>(value);
}

void expected_ycc(uint8 r, uint8 g, uint8 b, uint8* out) {
    out[0] = static_cast<uint8>((r * 19595 + g * 38470 + b * 7471 + 32768) >> 16);
    out[1] = clamp_ref(128 + ((r * -11059 + g * -21709 + b * 32768 + 32768) >> 16));
    out[2] = clamp_ref(128 + ((r * 32768 + g * -27439 + b * -5329 + 32768) >> 16));
}

uint8 expected_y(uint8 r, uint8 g, uint8 b) {
    return static_cast<uint8>((r * 19595 + g * 38470 + b * 7471 + 32768) >> 16);
}

void check(const std::string& name, const std::vector<uint8>& actual,
           const std::vector<uint8>& expected) {
    if (actual == expected) {
        ++passed;
        std::cout << "[PASS] " << name << '\n';
    } else {
        ++failed;
        std::cout << "[FAIL] " << name << "\n  expected:";
        for (auto v : expected) std::cout << ' ' << static_cast<int>(v);
        std::cout << "\n  actual:  ";
        for (auto v : actual) std::cout << ' ' << static_cast<int>(v);
        std::cout << '\n';
    }
}

void test_rgb_to_ycc() {
    struct Case { const char* name; std::vector<uint8> src; };
    const Case cases[] = {
        {"RGB_to_YCC black", {0,0,0}},
        {"RGB_to_YCC white", {255,255,255}},
        {"RGB_to_YCC red", {255,0,0}},
        {"RGB_to_YCC green", {0,255,0}},
        {"RGB_to_YCC blue", {0,0,255}},
        {"RGB_to_YCC gray", {128,128,128}},
        {"RGB_to_YCC cyan", {0,255,255}},
        {"RGB_to_YCC magenta", {255,0,255}},
        {"RGB_to_YCC yellow", {255,255,0}},
        {"RGB_to_YCC multiple pixels", {12,34,56, 78,90,123, 200,150,100}}
    };
    for (const auto& c : cases) {
        const int count = static_cast<int>(c.src.size() / 3);
        std::vector<uint8> actual(count * 3), expected(count * 3);
        for (int i = 0; i < count; ++i)
            expected_ycc(c.src[i*3], c.src[i*3+1], c.src[i*3+2], &expected[i*3]);
        jpge::RGB_to_YCC(actual.data(), c.src.data(), count);
        check(c.name, actual, expected);
    }
}

void test_rgb_to_y() {
    struct Case { const char* name; std::vector<uint8> src; };
    const Case cases[] = {
        {"RGB_to_Y black", {0,0,0}},
        {"RGB_to_Y white", {255,255,255}},
        {"RGB_to_Y red", {255,0,0}},
        {"RGB_to_Y green", {0,255,0}},
        {"RGB_to_Y blue", {0,0,255}},
        {"RGB_to_Y gray", {128,128,128}},
        {"RGB_to_Y cyan", {0,255,255}},
        {"RGB_to_Y magenta", {255,0,255}},
        {"RGB_to_Y yellow", {255,255,0}},
        {"RGB_to_Y multiple pixels", {12,34,56, 78,90,123, 200,150,100}}
    };
    for (const auto& c : cases) {
        const int count = static_cast<int>(c.src.size() / 3);
        std::vector<uint8> actual(count), expected(count);
        for (int i = 0; i < count; ++i)
            expected[i] = expected_y(c.src[i*3], c.src[i*3+1], c.src[i*3+2]);
        jpge::RGB_to_Y(actual.data(), c.src.data(), count);
        check(c.name, actual, expected);
    }
}

void test_rgba_to_ycc() {
    struct Case { const char* name; std::vector<uint8> src; };
    const Case cases[] = {
        {"RGBA_to_YCC black opaque", {0,0,0,255}},
        {"RGBA_to_YCC white opaque", {255,255,255,255}},
        {"RGBA_to_YCC red", {255,0,0,255}},
        {"RGBA_to_YCC green", {0,255,0,255}},
        {"RGBA_to_YCC blue", {0,0,255,255}},
        {"RGBA_to_YCC gray", {128,128,128,255}},
        {"RGBA_to_YCC transparent black", {0,0,0,0}},
        {"RGBA_to_YCC transparent color", {255,20,90,0}},
        {"RGBA_to_YCC semi-transparent", {45,120,210,127}},
        {"RGBA_to_YCC multiple pixels", {12,34,56,0, 78,90,123,127, 200,150,100,255}}
    };
    for (const auto& c : cases) {
        const int count = static_cast<int>(c.src.size() / 4);
        std::vector<uint8> actual(count * 3), expected(count * 3);
        for (int i = 0; i < count; ++i)
            expected_ycc(c.src[i*4], c.src[i*4+1], c.src[i*4+2], &expected[i*3]);
        jpge::RGBA_to_YCC(actual.data(), c.src.data(), count);
        check(c.name, actual, expected);
    }
}

void test_rgba_to_y() {
    struct Case { const char* name; std::vector<uint8> src; };
    const Case cases[] = {
        {"RGBA_to_Y black", {0,0,0,255}},
        {"RGBA_to_Y white", {255,255,255,255}},
        {"RGBA_to_Y red", {255,0,0,255}},
        {"RGBA_to_Y green", {0,255,0,255}},
        {"RGBA_to_Y blue", {0,0,255,255}},
        {"RGBA_to_Y gray", {128,128,128,255}},
        {"RGBA_to_Y transparent black", {0,0,0,0}},
        {"RGBA_to_Y transparent color", {255,20,90,0}},
        {"RGBA_to_Y semi-transparent", {45,120,210,127}},
        {"RGBA_to_Y multiple pixels", {12,34,56,0, 78,90,123,127, 200,150,100,255}}
    };
    for (const auto& c : cases) {
        const int count = static_cast<int>(c.src.size() / 4);
        std::vector<uint8> actual(count), expected(count);
        for (int i = 0; i < count; ++i)
            expected[i] = expected_y(c.src[i*4], c.src[i*4+1], c.src[i*4+2]);
        jpge::RGBA_to_Y(actual.data(), c.src.data(), count);
        check(c.name, actual, expected);
    }
}

void test_y_to_ycc() {
    struct Case { const char* name; std::vector<uint8> src; };
    const Case cases[] = {
        {"Y_to_YCC black", {0}},
        {"Y_to_YCC white", {255}},
        {"Y_to_YCC middle gray", {128}},
        {"Y_to_YCC low gray", {1}},
        {"Y_to_YCC high gray", {254}},
        {"Y_to_YCC two pixels", {0,255}},
        {"Y_to_YCC three pixels", {10,20,30}},
        {"Y_to_YCC gradient", {0,64,128,192,255}},
        {"Y_to_YCC repeated values", {77,77,77,77}},
        {"Y_to_YCC full range", {0,1,127,128,254,255}}
    };
    for (const auto& c : cases) {
        const int count = static_cast<int>(c.src.size());
        std::vector<uint8> actual(count * 3), expected(count * 3);
        for (int i = 0; i < count; ++i) {
            expected[i*3] = c.src[i];
            expected[i*3+1] = 128;
            expected[i*3+2] = 128;
        }
        jpge::Y_to_YCC(actual.data(), c.src.data(), count);
        check(c.name, actual, expected);
    }
}
} // namespace

int main() {
    test_rgb_to_ycc();
    test_rgb_to_y();
    test_rgba_to_ycc();
    test_rgba_to_y();
    test_y_to_ycc();
    std::cout << "\nTotal: " << (passed + failed) << ", passed: " << passed
              << ", failed: " << failed << '\n';
    return failed == 0 ? 0 : 1;
}
