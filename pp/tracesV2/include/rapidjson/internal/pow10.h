// Tencent is pleased to support the open source community by making RapidJSON available.
// 
// Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip. All rights reserved.
//
// Licensed under the MIT License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// http://opensource.org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, software distributed 
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
// CONDITIONS OF ANY KIND, either express or implied. See the License for the 
// specific language governing permissions and limitations under the License.

#ifndef RAPIDJSON_POW10_
#define RAPIDJSON_POW10_

#include "../rapidjson.h"

RAPIDJSON_NAMESPACE_BEGIN
namespace internal {

//! Computes integer powers of 10 in double (10.0^n).
/*! This function uses lookup table for fast and accurate results.
    \param n non-negative exponent. Must <= 308.
    \return 10.0^n
*/
inline double Pow10(int n) {
    static const double e[] = { // 1e-0...1e308: 309 * 8 bytes = 2472 bytes
        1e+0L,  
        1e+1L,  1e+2L,  1e+3L,  1e+4L,  1e+5L,  1e+6L,  1e+7L,  1e+8L,  1e+9L,  1e+10L, 1e+11L, 1e+12L, 1e+13L, 1e+14L, 1e+15L, 1e+16L, 1e+17L, 1e+18L, 1e+19L, 1e+20L, 
        1e+21L, 1e+22L, 1e+23L, 1e+24L, 1e+25L, 1e+26L, 1e+27L, 1e+28L, 1e+29L, 1e+30L, 1e+31L, 1e+32L, 1e+33L, 1e+34L, 1e+35L, 1e+36L, 1e+37L, 1e+38L, 1e+39L, 1e+40L,
        1e+41L, 1e+42L, 1e+43L, 1e+44L, 1e+45L, 1e+46L, 1e+47L, 1e+48L, 1e+49L, 1e+50L, 1e+51L, 1e+52L, 1e+53L, 1e+54L, 1e+55L, 1e+56L, 1e+57L, 1e+58L, 1e+59L, 1e+60L,
        1e+61L, 1e+62L, 1e+63L, 1e+64L, 1e+65L, 1e+66L, 1e+67L, 1e+68L, 1e+69L, 1e+70L, 1e+71L, 1e+72L, 1e+73L, 1e+74L, 1e+75L, 1e+76L, 1e+77L, 1e+78L, 1e+79L, 1e+80L,
        1e+81L, 1e+82L, 1e+83L, 1e+84L, 1e+85L, 1e+86L, 1e+87L, 1e+88L, 1e+89L, 1e+90L, 1e+91L, 1e+92L, 1e+93L, 1e+94L, 1e+95L, 1e+96L, 1e+97L, 1e+98L, 1e+99L, 1e+100L,
        1e+101L,1e+102L,1e+103L,1e+104L,1e+105L,1e+106L,1e+107L,1e+108L,1e+109L,1e+110L,1e+111L,1e+112L,1e+113L,1e+114L,1e+115L,1e+116L,1e+117L,1e+118L,1e+119L,1e+120L,
        1e+121L,1e+122L,1e+123L,1e+124L,1e+125L,1e+126L,1e+127L,1e+128L,1e+129L,1e+130L,1e+131L,1e+132L,1e+133L,1e+134L,1e+135L,1e+136L,1e+137L,1e+138L,1e+139L,1e+140L,
        1e+141L,1e+142L,1e+143L,1e+144L,1e+145L,1e+146L,1e+147L,1e+148L,1e+149L,1e+150L,1e+151L,1e+152L,1e+153L,1e+154L,1e+155L,1e+156L,1e+157L,1e+158L,1e+159L,1e+160L,
        1e+161L,1e+162L,1e+163L,1e+164L,1e+165L,1e+166L,1e+167L,1e+168L,1e+169L,1e+170L,1e+171L,1e+172L,1e+173L,1e+174L,1e+175L,1e+176L,1e+177L,1e+178L,1e+179L,1e+180L,
        1e+181L,1e+182L,1e+183L,1e+184L,1e+185L,1e+186L,1e+187L,1e+188L,1e+189L,1e+190L,1e+191L,1e+192L,1e+193L,1e+194L,1e+195L,1e+196L,1e+197L,1e+198L,1e+199L,1e+200L,
        1e+201L,1e+202L,1e+203L,1e+204L,1e+205L,1e+206L,1e+207L,1e+208L,1e+209L,1e+210L,1e+211L,1e+212L,1e+213L,1e+214L,1e+215L,1e+216L,1e+217L,1e+218L,1e+219L,1e+220L,
        1e+221L,1e+222L,1e+223L,1e+224L,1e+225L,1e+226L,1e+227L,1e+228L,1e+229L,1e+230L,1e+231L,1e+232L,1e+233L,1e+234L,1e+235L,1e+236L,1e+237L,1e+238L,1e+239L,1e+240L,
        1e+241L,1e+242L,1e+243L,1e+244L,1e+245L,1e+246L,1e+247L,1e+248L,1e+249L,1e+250L,1e+251L,1e+252L,1e+253L,1e+254L,1e+255L,1e+256L,1e+257L,1e+258L,1e+259L,1e+260L,
        1e+261L,1e+262L,1e+263L,1e+264L,1e+265L,1e+266L,1e+267L,1e+268L,1e+269L,1e+270L,1e+271L,1e+272L,1e+273L,1e+274L,1e+275L,1e+276L,1e+277L,1e+278L,1e+279L,1e+280L,
        1e+281L,1e+282L,1e+283L,1e+284L,1e+285L,1e+286L,1e+287L,1e+288L,1e+289L,1e+290L,1e+291L,1e+292L,1e+293L,1e+294L,1e+295L,1e+296L,1e+297L,1e+298L,1e+299L,1e+300L,
        1e+301L,1e+302L,1e+303L,1e+304L,1e+305L,1e+306L,1e+307L,1e+308L
    };
    RAPIDJSON_ASSERT(n >= 0 && n <= 308);
    return e[n];
}

} // namespace internal
RAPIDJSON_NAMESPACE_END

#endif // RAPIDJSON_POW10_
