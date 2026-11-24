#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "jpge_internal_modules.h"

namespace {

int passed = 0;
int failed = 0;

const int BLOCK_SIZE = 8;

void check(const std::string& name, const std::vector<jpge::int32>& actual,
           const std::vector<jpge::int32>& expected) {
    bool equal = actual.size() == expected.size();

    if (equal) {
        for (std::size_t i = 0; i < actual.size(); ++i) {
            int difference = static_cast<int>(actual[i] - expected[i]);
            if (difference < -1 || difference > 1) {
                equal = false;
                break;
            }
        }
    }

    if (equal) {
        ++passed;
        std::cout << "[PASS] " << name << '\n';
    } else {
        ++failed;
        std::cout << "[FAIL] " << name << "\n  expected:";
        for (std::size_t i = 0; i < expected.size(); ++i)
            std::cout << ' ' << expected[i];
        std::cout << "\n  actual:  ";
        for (std::size_t i = 0; i < actual.size(); ++i)
            std::cout << ' ' << actual[i];
        std::cout << '\n';
    }
}

std::vector<jpge::int32> reference_dct(const std::vector<jpge::int32>& input) {
    std::vector<jpge::int32> result(BLOCK_SIZE * BLOCK_SIZE, 0);
    const double pi = std::acos(-1.0);

    for (int v = 0; v < BLOCK_SIZE; ++v) {
        for (int u = 0; u < BLOCK_SIZE; ++u) {
            double sum = 0.0;

            for (int y = 0; y < BLOCK_SIZE; ++y) {
                for (int x = 0; x < BLOCK_SIZE; ++x) {
                    double horizontal = std::cos((2.0 * x + 1.0) * u * pi / 16.0);
                    double vertical = std::cos((2.0 * y + 1.0) * v * pi / 16.0);
                    sum += input[y * BLOCK_SIZE + x] * horizontal * vertical;
                }
            }

            double cu = (u == 0) ? 1.0 / std::sqrt(2.0) : 1.0;
            double cv = (v == 0) ? 1.0 / std::sqrt(2.0) : 1.0;
            double value = 0.25 * cu * cv * sum;
            result[v * BLOCK_SIZE + u] = static_cast<jpge::int32>(std::lround(value));
        }
    }

    return result;
}

void run_test(const std::string& name, const std::vector<jpge::int32>& input) {
    std::vector<jpge::int32> actual = input;
    std::vector<jpge::int32> expected = reference_dct(input);

    jpge::DCT2D(actual.data());
    check(name, actual, expected);
}

std::vector<jpge::int32> make_block(int (*value)(int, int)) {
    std::vector<jpge::int32> block(BLOCK_SIZE * BLOCK_SIZE);
    for (int y = 0; y < BLOCK_SIZE; ++y)
        for (int x = 0; x < BLOCK_SIZE; ++x)
            block[y * BLOCK_SIZE + x] = value(x, y);
    return block;
}

int zero(int, int) { return 0; }
int constant_ten(int, int) { return 10; }
int constant_negative(int, int) { return -25; }
int horizontal_ramp(int x, int) { return x * 8; }
int vertical_ramp(int, int y) { return y * 8; }
int diagonal_ramp(int x, int y) { return (x + y) * 4; }
int reverse_ramp(int x, int y) { return (14 - x - y) * 4; }
int checkerboard(int x, int y) { return ((x + y) % 2) ? 100 : -100; }
int horizontal_alternation(int x, int) { return (x % 2) ? 80 : -80; }
int vertical_alternation(int, int y) { return (y % 2) ? 80 : -80; }
int center_impulse(int x, int y) { return (x == 4 && y == 4) ? 255 : 0; }
int corner_impulse(int x, int y) { return (x == 0 && y == 0) ? 255 : 0; }
int opposite_corners(int x, int y) {
    return ((x == 0 && y == 0) || (x == 7 && y == 7)) ? 200 : 0;
}
int left_half(int x, int) { return x < 4 ? 120 : -120; }
int top_half(int, int y) { return y < 4 ? 120 : -120; }
int quadrant_pattern(int x, int y) { return ((x < 4) == (y < 4)) ? 90 : -90; }
int three_levels(int x, int y) { return ((x + y) % 3 - 1) * 70; }
int low_values(int x, int y) { return (x * 3 + y * 5) % 16 - 8; }
int high_positive(int x, int y) { return 200 + ((x * 7 + y * 11) % 56); }
int high_negative(int x, int y) { return -200 - ((x * 7 + y * 11) % 56); }
int mixed_signs(int x, int y) { return ((x * 13 + y * 7) % 2) ? 127 : -128; }
int diagonal_line(int x, int y) { return x == y ? 180 : 0; }
int border_pattern(int x, int y) {
    return (x == 0 || x == 7 || y == 0 || y == 7) ? 100 : 0;
}
int deterministic_noise(int x, int y) { return ((x * 37 + y * 19 + x * y * 3) % 255) - 127; }

} // namespace

int main() {
    run_test("DCT zero block", make_block(zero));
    run_test("DCT constant positive block", make_block(constant_ten));
    run_test("DCT constant negative block", make_block(constant_negative));
    run_test("DCT horizontal ramp", make_block(horizontal_ramp));
    run_test("DCT vertical ramp", make_block(vertical_ramp));
    run_test("DCT diagonal ramp", make_block(diagonal_ramp));
    run_test("DCT reverse diagonal ramp", make_block(reverse_ramp));
    run_test("DCT checkerboard", make_block(checkerboard));
    run_test("DCT horizontal alternation", make_block(horizontal_alternation));
    run_test("DCT vertical alternation", make_block(vertical_alternation));
    run_test("DCT center impulse", make_block(center_impulse));
    run_test("DCT corner impulse", make_block(corner_impulse));
    run_test("DCT opposite corner impulses", make_block(opposite_corners));
    run_test("DCT left and right halves", make_block(left_half));
    run_test("DCT top and bottom halves", make_block(top_half));
    run_test("DCT quadrant pattern", make_block(quadrant_pattern));
    run_test("DCT three-level pattern", make_block(three_levels));
    run_test("DCT low values", make_block(low_values));
    run_test("DCT high positive values", make_block(high_positive));
    run_test("DCT high negative values", make_block(high_negative));
    run_test("DCT mixed signs", make_block(mixed_signs));
    run_test("DCT diagonal line", make_block(diagonal_line));
    run_test("DCT border pattern", make_block(border_pattern));
    run_test("DCT deterministic noise", make_block(deterministic_noise));

    std::cout << "\nTotal: " << (passed + failed) << ", passed: " << passed
              << ", failed: " << failed << '\n';
    return failed == 0 ? 0 : 1;
}
