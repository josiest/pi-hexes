#pragma once

// math and algorithms
#include <cmath>        // abs, sqrt, sqrtf
#include <algorithm>    // max_element
#include <numeric>

// containers and types
#include <array>
#include <iterator>

#include <pi/geometry.hpp>

namespace pi
{
enum class HexTop{ Pointed, Flat };

template<std::floating_point Field>
static constexpr Field sqrt3 = 1.73205;

template<euclidean_vector2 Point>
constexpr scalar_field_t<Point> hex_q_value(const Point & p) { return p.x; }

template<euclidean_vector2 Point>
constexpr scalar_field_t<Point> hex_r_value(const Point & p) { return p.y; }

template<euclidean_vector2 Point>
constexpr scalar_field_t<Point> hex_s_value(const Point & p) { return -p.x - p.y; }


/**
 * Compute the affine basis vectors for hex-space in 2 dimensions
 */
template<std::floating_point Field>
affine_transform2<Field> hex_basis2(HexTop top_style)
{
    const Field x_offset_angle = top_style == HexTop::Pointed? 0.f : std::numbers::pi_v<Field>/6.f;
    const Field y_offset_angle = top_style == HexTop::Pointed? std::numbers::pi_v<Field>/3.f
                                                             : std::numbers::pi_v<Field>/2.f;

    affine_transform2<Field> basis;
    basis.x_basis(std::cos(x_offset_angle), std::sin(x_offset_angle));
    basis.y_basis(std::cos(y_offset_angle), std::sin(y_offset_angle));
    basis.scale_by(std::sqrt(3.f));

    return basis;
}

/**
 * Calculate the hex norm of h.
 *
 * Equivalent to \f$\frac{|h_q| + |h_r| + |h_s|}{2}\f$
 */
template<euclidean_vector2 Point>
scalar_field_t<Point> hex_norm(const Point & p) noexcept
{
    return (std::abs(p.x) + std::abs(p.y) + std::abs(-p.x-p.y))/2;
}


/**
 * Calculate the hex with the minimum distance to `p` who's components are integers.
 */
template<euclidean_vector2 OutPoint, numeric InField>
requires std::floating_point<InField>
OutPoint nearest_hex(InField x, InField y)
{
    vec2 hex_values{ x, y, -x-y };
    auto rounded_hex_values = hex_values;

    std::ranges::transform(hex_values, std::begin(rounded_hex_values),
                           [](const auto e) { return std::round(e); });

    vec2 differences{ std::valarray(std::abs(rounded_hex_values.data - hex_values.data)) };

    auto i = std::distance(std::begin(differences), std::ranges::max_element(differences));
    rounded_hex_values.data[i] -= std::accumulate(std::begin(rounded_hex_values),
                                                  std::end(rounded_hex_values), InField(0));

    using OutField = scalar_field_t<OutPoint>;
    return OutPoint{ static_cast<OutField>(rounded_hex_values.data[0]),
                     static_cast<OutField>(rounded_hex_values.data[1]) };
}
template<euclidean_vector2 OutPoint, euclidean_vector2 InPoint>
requires std::floating_point<scalar_field_t<InPoint>>
OutPoint nearest_hex(const InPoint & p)
{
    return nearest_hex<OutPoint>(p.x, p.y);
}

template<euclidean_vector2 Point>
requires std::floating_point<scalar_field_t<Point>>
Point lerp(const Point & a, const Point & b, scalar_field_t<Point> t)
{
    return Point{ std::lerp(a.x, b.x, t), std::lerp(a.y, b.y, t) };
}

/**
 * Calculate the hex coordinates in a "straight" line segment from `a` to `b`.
 *
 * The coordinates are calculated by finding the the nearest hex coordinates
 * with integer components to the line segment between a and b.
 */
template<euclidean_vector2 Point, std::output_iterator<Point> PointOutput>
requires std::integral<scalar_field_t<Point>>
PointOutput line(const Point& a, const Point& b, PointOutput into_hexes) noexcept
{
    const auto ax = static_cast<float>(a.x); const auto ay = static_cast<float>(a.y);
    const auto bx = static_cast<float>(b.x); const auto by = static_cast<float>(b.y);

    const scalar_field_t<Point> n = hex_norm(Point{ a.x-b.x, a.y-b.y });
    *into_hexes++ = a;
    for (int i = 1; i < n; i++)
    {
        const float t = static_cast<float>(i)/static_cast<float>(n);
        *into_hexes++ = nearest_hex<Point>(std::lerp(ax, bx, t), std::lerp(ay, by, t));
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
template<euclidean_vector2 Point, std::output_iterator<Point> PointOutput>
PointOutput hex_range(const Point& center, scalar_field_t<Point> r, PointOutput into_hexes)
{
    using Field = scalar_field_t<Point>;
    for (Field i = -r; i <= r; i += Field(1))
    {
        for (Field j = std::max(-r, -r-i); j <= std::min(r, r-i); j += Field(1))
        {
            *into_hexes++ = center + Point{i, j};
        }
    }
    return into_hexes;
}

/** Calculate the vertices of `p` in cartesian space */
template<euclidean_vector2 Point, std::output_iterator<Point> PointOutput>
requires std::floating_point<scalar_field_t<Point>>
PointOutput hex_vertices(HexTop hex_style, PointOutput into_verts)
{
    using Field = scalar_field_t<Point>;
    static constexpr Field pi = std::numbers::pi_v<Field>;

    const Field offset = hex_style == HexTop::Pointed? pi/6 : 0;

    // add each vertex to the list
    for (int i = 0; i < 6; ++i)
    {
        // calculate the angle of the vertex
        Field theta = offset + i * pi/3;

        // convert the angle to unit vector, then scale and offset
        *into_verts++ = Point{ std::cos(theta), std::sin(theta) };
    }
    return into_verts;
}
}