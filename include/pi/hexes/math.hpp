#pragma once

#include <pi/geometry.hpp>
#include <concepts>

namespace pi
{
template<typename T> using q_field_t = std::remove_cvref_t<decltype(std::declval<T>().q)>;
template<typename T> using r_field_t = std::remove_cvref_t<decltype(std::declval<T>().r)>;

template<typename Point>
concept axial = requires(Point p)
{
    Point{};
    { p.q } -> numeric;
    { p.r } -> std::convertible_to<q_field_t<Point>>;
    Point{ p.q, p.r };
};

template<typename Point>
concept coordinate = axial<Point> or euclidean_vector2<Point>;

template<axial Point>
struct scalar_field<Point>
{
    using type = q_field_t<Point>;
};
}
