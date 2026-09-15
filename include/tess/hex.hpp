#pragma once

#include <cmath>        // abs, sqrt, sqrtf
#include <algorithm>    // max_element
#include <numeric>
#include <valarray>
#include <iterator>
#include <ranges>

#include "math.hpp"
#include <tuple>
#include <cstddef>

namespace tess {

/**
 * A representation of a hexagonal coordinate.
 *
 * Hexes may be printed, added, subtracted and negated
 *
 * \code{.cpp}
 * hex<int> const h1(1, 2);
 * hex<int> const h2(3, 4);
 *
 * std::cout << (h1+h2) << '\n';
 * std::cout << (h1-h2) << '\n';
 * std::cout << (-h1) << '\n';
 * \endcode
 *
 * ```
 * > <4, 6, -10>
 * > <-2, -2, 4>
 * > <-1, -2, 3>
 * ```
 */
template<numeric Field>
struct hex
{
    Field q, r;
};

template<numeric Field>
hex(Field, Field) -> hex<Field>;

template<axial Point>
constexpr scalar_field_t<Point> hex_q_value(const Point & p) { return p.q; }

template<axial Point>
constexpr scalar_field_t<Point> hex_r_value(const Point & p) { return p.r; }

template<axial Point>
constexpr scalar_field_t<Point> hex_s_value(const Point & p) { return -p.q - p.r; }

template<cartesian Point>
constexpr scalar_field_t<Point> hex_q_value(const Point & p) { return p.x; }

template<cartesian Point>
constexpr scalar_field_t<Point> hex_r_value(const Point & p) { return p.y; }

template<cartesian Point>
constexpr scalar_field_t<Point> hex_s_value(const Point & p) { return -p.x - p.y; }

/**
 * Calculate the hex norm of h.
 *
 * Equivalent to \f$\frac{|h_q| + |h_r| + |h_s|}{2}\f$
 */
template<coordinate Point>
scalar_field_t<Point> hex_norm(const Point & p) noexcept
{
    return (std::abs(hex_q_value(p)) + std::abs(hex_r_value(p)) + std::abs(hex_s_value(p)))/2;
}


/**
 * Calculate the hex with the minimum distance to `h` who's components are
 * integers.
 */
template<coordinate OutPoint, coordinate InPoint>
OutPoint nearest_hex(const InPoint & p)
{
    using InField = scalar_field_t<InPoint>;
    std::valarray<InField> hex_values{ hex_q_value(p), hex_r_value(p), hex_s_value(p) };

    auto rounded_hex_values = hex_values;
    std::ranges::transform(hex_values, std::begin(rounded_hex_values),
                           [](const auto e) { return std::round(e); });

    std::valarray<InField> differences{ std::abs(rounded_hex_values - hex_values) };

    auto i = std::distance(std::begin(differences), std::ranges::max_element(differences));
    rounded_hex_values[i] -= std::accumulate(std::begin(rounded_hex_values), std::end(rounded_hex_values), 0.f);

    using OutField = scalar_field_t<OutPoint>;
    return OutPoint{ static_cast<OutField>(rounded_hex_values[0]),
                     static_cast<OutField>(rounded_hex_values[1]) };
}

template<typename Point>
requires (axial<Point> or cartesian<Point>) and std::floating_point<scalar_field_t<Point>>
Point lerp(const Point & a, const Point & b, scalar_field_t<Point> t)
{
    if constexpr (axial<Point>)
    {
        return Point{ std::lerp(a.q, b.q, t), std::lerp(a.r, b.r, t) };
    }
    else
    {
        return Point{ std::lerp(a.x, b.x, t), std::lerp(a.y, b.y, t) };
    }
}

/**
 * Calculate the hex coordinates in a "straight" line segment from `a` to `b`.
 *
 * The coordinates are calculated by finding the the nearest hex coordinates
 * with integer components to the line segment between a and b.
 */
template<coordinate Hex, std::output_iterator<Hex> HexOutput>
requires std::integral<scalar_field_t<Hex>>
HexOutput line(const Hex& a, const Hex& b, HexOutput into_hexes) noexcept
{
    using Integer = scalar_field_t<Hex>;
    const hex real_a
    {
        .q = static_cast<float>(hex_q_value(a)),
        .r = static_cast<float>(hex_r_value(a))
    };
    const hex real_b
    {
        .q = static_cast<float>(hex_q_value(b)),
        .r = static_cast<float>(hex_r_value(b))
    };

    const Hex difference
    {
        hex_q_value(a)-hex_q_value(b),
        hex_r_value(a)-hex_r_value(b)
    };

    const Integer n = hex_norm(difference);
    *into_hexes++ = a;
    for (int i = 1; i < n; i++)
    {
        const float t = static_cast<float>(i)/static_cast<float>(n);
        *into_hexes++ = nearest_hex<Hex>(lerp(real_a, real_b, t));
    }
    *into_hexes++ = b;
    return into_hexes;
}

/**
 * Calculate the set of hex coordinates within radius `r` of `center`.
 *
 * \note A hex coordinate `a` is within radius `r` of `b` if
 *       `hex_norm(a-b) <= r`.
 */
template<typename Hex, std::output_iterator<Hex> HexOutput>
requires (axial<Hex> or cartesian<Hex>) and std::integral<scalar_field_t<Hex>>
HexOutput hex_range(const Hex& center, scalar_field_t<Hex> r, HexOutput into_hexes)
{
    using Integer = scalar_field_t<Hex>;
    for (Integer i = -r; i <= r; ++i)
    {
        for (Integer j = std::max(-r, -r-i); j <= std::min(r, r-i); ++j)
        {
            *into_hexes++ = center + Hex{i, j};
        }
    }
    return into_hexes;
}

template<numeric Field>
bool operator==(const hex<Field>& a, const hex<Field>& b)
{
    return a.q == b.q and a.r == b.r;
}

template<numeric Field>
hex<Field> operator+(const hex<Field>& a, const hex<Field>& b)
{
    return hex<Field>{a.q+b.q, a.r+b.r};
}

template<numeric Field>
hex<Field> operator-(const hex<Field>& h)
{
    return hex<Field>{-h.q, -h.r};
}

template<numeric Field>
hex<Field> operator-(const hex<Field>& a, const hex<Field>& b)
{
    return a + (-b);
}
}
//
// tuple size
//

template<typename Field>
struct std::tuple_size<tess::hex<Field>> {
    static constexpr const std::size_t value = 2;
};

//
// tuple element types
//

template<std::size_t i, typename Field>
requires (i < 2)
struct std::tuple_element<i, tess::hex<Field>> {
    using type = std::add_const_t<Field>;
};

template <tess::numeric Field>
struct std::hash<tess::hex<Field>> {
    size_t operator()(const tess::hex<Field>& h) const
    {
        constexpr hash<double> double_hash;
        size_t hq = double_hash(static_cast<double>(h.q));
        size_t hr = double_hash(static_cast<double>(h.r));
        return hq ^ (hr + 0x9e3779b9 + (hq << 6) + (hq >> 2));
    }
};
