/**
* \file ZigguratNormal.cpp
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Implementation for the Ziggurat algorithm to approximate a normal distribution (taken from https://www.jstatsoft.org/article/download/v005i08/623)
* \date 2022-04-24
*
* MIT License
* Copyright (c) [2022] [Weckyy702 (weckyy702@gmail.com | https://github.com/Weckyy702)]
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/

#include "Raychel/Core/ZigguratNormal.h"
#include "Raychel/Core/Xoroshiro128+.h"

#include "RaychelCore/Raychel_assert.h"
#include "RaychelMath/math.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <random>

namespace Raychel {

    namespace details {

        constexpr auto R = 3.6541528853610088;

        constexpr static std::array X_table{
            3.7130862467425505,  3.442619855899,      3.2230849845811416,
            3.0832288582168683,  2.9786962526477803,  2.894344007021529,
            2.8231253505489105,  2.761169372387177,   2.7061135731218195,
            2.6564064112613597,  2.6109722484318474,  2.569033625924938,
            2.5300096723888275,  2.493454522095372,   2.4590181774118305,
            2.42642064553375,    2.3954342780110625,  2.3658713701176386,
            2.3375752413392368,  2.310413683698763,   2.2842740596774718,
            2.2590595738691985,  2.2346863955909795,  2.2110814088787034,
            2.188180432076049,   2.165926793748922,   2.1442701823603953,
            2.1231657086739766,  2.1025731351892385,  2.082456237992017,
            2.0627822745083084,  2.0435215366550676,  2.0246469733773855,
            2.006133869963472,   1.98795957412762,    1.9701032608543265,
            1.9525457295535567,  1.9352692282966228,  1.9182573008645099,
            1.901494653105151,   1.884967035707759,   1.8686611409944887,
            1.8525645117280911,  1.836665460258446,   1.8209529965961255,
            1.8054167642192285,  1.7900469825998586,  1.7748343955860695,
            1.7597702248995934,  1.7448461281138004,  1.7300541605637305,
            1.7153867407136676,  1.7008366185699169,  1.6863968467791681,
            1.672060754097601,   1.6578219209540241,  1.6436741568628686,
            1.6296114794706347,  1.615628095043161,   1.6017183802213781,
            1.5878768648905761,  1.5740982160230008,  1.560377222366169,
            1.5467087798599104,  1.5330878776740433,  1.5195095847659401,
            1.5059690368632033,  1.492461423781354,   1.4789819769899242,
            1.4655259573427108,  1.4520886428892246,  1.4386653166845635,
            1.42525125451406,    1.4118417124470577,  1.3984319141310053,
            1.3850170377326518,  1.3715922024273426,  1.3581524543301435,
            1.344692751753547,   1.3312079496656273,  1.317692783209414,
            1.3041418501286168,  1.2905495919261964,  1.2769102735601556,
            1.263217961454621,   1.2494664995730682,  1.2356494832633627,
            1.2217602305399964,  1.2077917504159497,  1.1937367078331287,
            1.1795873846639882,  1.1653356361647524,  1.1509728421488674,
            1.1364898520131608,  1.1218769225825422,  1.107123647534036,
            1.0922188769072774,  1.0771506248928957,  1.0619059636948243,
            1.0464709007640454,  1.0308302360681956,  1.0149673952513305,
            0.9988642334929836,  0.982500803515429,   0.9658550794011499,
            0.9489026255113064,  0.9316161966151508,  0.9139652510230323,
            0.8959153525809377,  0.8774274291129234,  0.8584568431938132,
            0.8389522142975774,  0.8188539067003573,  0.7980920606440569,
            0.7765839878947599,  0.7542306644540556,  0.7309119106424888,
            0.7064796113354365,  0.6807479186691546,  0.6534786387399752,
            0.6243585973360507,  0.5929629424714483,  0.5586921784081852,
            0.5206560387620606,  0.4774378372966898,  0.4265479863554235,
            0.36287143109703196, 0.27232086481396467, 0.0,
        };

        constexpr static std::array R_table{
            0.9271586026096681, 0.9362302895738892, 0.9566079929529229, 0.9660963845448882,
            0.971681487982781,  0.9753938521821022, 0.9780541171685178, 0.980060694640489,
            0.9816315315239645, 0.9828963811271866, 0.9839375456663325, 0.9848098704733534,
            0.9855513792328944, 0.9861893030819736, 0.9867436799867864, 0.9872295978111943,
            0.9876586437103296, 0.9880398701570176, 0.9883804563121089, 0.9886861715693078,
            0.9889617072428545, 0.9892109183130244, 0.9894370025436909, 0.9896426351781105,
            0.9898300715969688, 0.9900012265183524, 0.9901577357834697, 0.9903010050508025,
            0.9904322485336944, 0.9905525200843218, 0.9906627383358567, 0.9907637071892196,
            0.9908561326209719, 0.9909406365607181, 0.991017768416579,  0.9910880146997187,
            0.991151807102165,  0.991209529308185,  0.9912615227624552, 0.9913080915739614,
            0.9913495066999154, 0.9913860095266759, 0.9914178149430195, 0.9914451139838447,
            0.9914680761085329, 0.9914868511670121, 0.9915015710974835, 0.9915123513923666,
            0.9915192923629307, 0.9915224802280646, 0.9915219880484646, 0.9915178765240442,
            0.9915101946694387, 0.9914989803800052, 0.9914842608986051, 0.9914660531916395,
            0.9914443642412228, 0.9914191912590011, 0.9913905218258715, 0.9913583339607497,
            0.9913225961204966, 0.9912832671321499, 0.9912402960576856, 0.991193621990624,
            0.991143173782899,  0.991088869699481,  0.9910306169972894, 0.9909683114239041,
            0.9909018366304913, 0.9908310634921467, 0.9907558493275227, 0.9906760370080955,
            0.9905914539457294, 0.9905019109452362, 0.9904072009063883, 0.990307097357238,
            0.990201352797563,  0.9900896968277136, 0.9899718340339569, 0.9898474415964779,
            0.9897161665803526, 0.9895776228628198, 0.9894313876418468, 0.9892769974609422,
            0.9891139436730952, 0.9889416672520418, 0.9887595528412437, 0.9885669219091597,
            0.9883630248526034, 0.9881470318569457, 0.9879180222809051, 0.987674972282531,
            0.9874167403388364, 0.9871420502305995, 0.9868494709610887, 0.9865373929461655,
            0.986203999644239,  0.9858472335755389, 0.98546475539409,   0.9850538942989907,
            0.9846115875710347, 0.9841343063494573, 0.9836179638544746, 0.9830578010168337,
            0.9824482427525728, 0.9817827157061126, 0.9810534148544756, 0.9802510014227667,
            0.9793642073274506, 0.9783793105963312, 0.9772794298852922, 0.9760435609386315,
            0.9746452378300764, 0.9730506368752245, 0.9712158326862985, 0.9690827290502092,
            0.9665728537853818, 0.9635775863118795, 0.959942176565901,  0.9554384188286962,
            0.9497153478809163, 0.9422042060159378, 0.9319193267489506, 0.9169927970716931,
            0.8934105197245976, 0.8507165493794344, 0.7504610213889943, 0.0,
        };

        //NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
        static thread_local std::uint32_t xorshift32_seed{2463534242U};
        [[nodiscard]] std::uint32_t random32() noexcept
        {
            xorshift32_seed ^= (xorshift32_seed << 13U);
            xorshift32_seed ^= (xorshift32_seed >> 17U);
            xorshift32_seed ^= (xorshift32_seed << 5U);

            return xorshift32_seed;
        }

        //NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
        static thread_local std::uint64_t xorshift64_seed{88172645463325252U};
        [[nodiscard]] std::uint64_t random64() noexcept
        {
            xorshift64_seed ^= (xorshift64_seed << 13U);
            xorshift64_seed ^= (xorshift64_seed >> 7U);
            xorshift64_seed ^= (xorshift64_seed << 17U);

            return xorshift64_seed;
        }

        //Fast way to generate "uniform" reals in the range [0; 1]. Credit to Inigo Quilez (https://iquilezles.org/articles/sfrand/)
        [[nodiscard]] double uniform01() noexcept
        {
            //The idea is to generate 52 random bits for the mantissa and fix the exponent to 1023 to generate a random number between 1 and 2.
            //We can then easily map that number into the [0; 1] range by subtracting 1
            static_assert(sizeof(std::uint64_t) == sizeof(double));

            constexpr std::uint64_t fix_exponent{0x3FF0000000000000};

            const auto mantissa = (random64() >> 12U);

            return std::bit_cast<double>(mantissa | fix_exponent) - 1.0;
        }

        [[nodiscard]] static double normal_tail(bool is_negative) noexcept
        {
            double x{};
            double y{};
            std::size_t guard{};
            do {
                x = std::log(uniform01()) / R;
                y = std::log(uniform01());

                RAYCHEL_ASSERT(++guard != 16U);
            } while (-2 * y < sq(x));

            if (is_negative) {
                return x - R;
            }
            return R - x;
        }
    } // namespace details

    double uniform_random() noexcept
    {
        return details::uniform01() * 2 - 1;
    }

    double ziggurat_normal() noexcept
    {
        //The guard variable is just to prevent an infinte loop. It's not gonna matter in practice
        for (std::size_t guard{}; guard != 16U; ++guard) {
            const auto u = 2.0 * details::uniform01() - 1.0;
            const auto i = details::random32() & 0x7FU;
            /* first try the rectangular boxes */
            if (std::abs(u) < details::R_table.at(i)) {
                return u * details::X_table.at(i);
            }
            /* bottom box: sample from the tail */
            if (i == 0) {
                return details::normal_tail(u < 0);
            }
            /* is this a sample from the wedges? */
            const auto x = u * details::X_table.at(i);
            const auto f0 = std::exp(-0.5 * (sq(details::X_table.at(i)) - sq(x)));
            const auto f1 = std::exp(-0.5 * (sq(details::X_table.at(i + 1)) - sq(x)));
            if (f1 + details::uniform01() * (f0 - f1) < 1.0) {
                return x;
            }
        }
        RAYCHEL_ASSERT_NOT_REACHED;
    }

} //namespace Raychel
