#include "Raychel/Core/ZigguratNormal.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>

int zig_main()
{
    constexpr auto scale = 25.0;
    constexpr std::size_t n{100'000'000};
    constexpr auto expected0 = static_cast<std::size_t>(n * 0.04 / 100.0);

    std::map<std::int32_t, std::size_t> hist{};

    for (std::size_t i{}; i != n; ++i) {
        ++hist[static_cast<std::int32_t>(std::round(Raychel::ziggurat_normal() * scale))];
    }

    for (const auto& [value, count] : hist) {
        const auto actual_value = static_cast<double>(value) / scale;
        if (actual_value >= 0) {
            std::cout << ' ';
        }
        std::cout << std::fixed << std::setprecision(3) << actual_value << ": " << std::string(count / expected0, '*') << ">\n";
    }

    return 0;
}
