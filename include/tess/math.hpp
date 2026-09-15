#pragma once

#include <concepts>

namespace tess {
template<typename T>
concept numeric = std::integral<std::remove_cvref_t<T>> or std::floating_point<std::remove_cvref_t<T>>;

template<typename T> using x_field_t = std::remove_cvref_t<decltype(std::declval<T>().x)>;
template<typename T> using y_field_t = std::remove_cvref_t<decltype(std::declval<T>().y)>;

template<typename Point>
concept cartesian = requires(Point p)
{
    Point{};
    { p.x } -> numeric;
    { p.y } -> std::convertible_to<x_field_t<Point>>;
    Point{ p.x, p.y };
};

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
concept coordinate = axial<Point> or cartesian<Point>;

template<typename>
struct scalar_field
{
};

template<cartesian Point>
struct scalar_field<Point>
{
    using type = x_field_t<Point>;
};

template<axial Point>
struct scalar_field<Point>
{
    using type = q_field_t<Point>;
};

template<typename Point>
using scalar_field_t = scalar_field<Point>::type;
}
