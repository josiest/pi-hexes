#pragma once

#include <cmath>
#include <valarray>
#include <concepts>
#include <numbers>

#include "math.hpp"

namespace pi
{
enum class HexTop { Flat, Pointed };

/** An abstract data type for converting to and from screen and hex space. */
template<std::floating_point Real>
class Basis
{
public:
    /**
     * Create a basis centered at `origin` in screen space.
     *
     * Basis must have a positive `unit_size` measured in pixels. `top`
     * determines if the top of each hex unit is flat or pointed.
     */
    Basis(Real x, Real y, Real unit_size, HexTop top_style)

        : _basis{4}, _inverse{4}, _top_style(top_style),
          x{x}, y{y}, _unit_size{unit_size}
    {
        static constexpr Real sqrt3 = 1.73205;
        if (top_style == HexTop::Pointed)
        {
            _basis = {sqrt3, sqrt3/2, 0, 3/Real(2)};
            _inverse = {sqrt3/3, -1/Real(3), 0, 2/Real(3)};
        }
        else
        {
            _basis = {3/Real(2), 0, sqrt3/2, sqrt3};
            _inverse = {2/Real(3), 0, -1/Real(3), sqrt3/3};
        }
        _basis *= _unit_size;
        _inverse /= Real(_unit_size);
    }

    /** The origin of this basis in screen space (pixels). */
    template<cartesian Point>
    Point origin() const noexcept { return Point{x, y}; }

    /** The unit size of this basis in pixels. */
    Real unit_size() const noexcept { return _unit_size; }

    /** Convert `h` to a point in screen space. */
    template<cartesian Point, typename Hex>
    requires cartesian<Hex> or axial<Hex>
    Point pixel(Hex const & h) const noexcept
    {
        std::valarray<Real> hex_value(2);
        if constexpr (axial<Hex>) { hex_value[0] = static_cast<Real>(h.q); hex_value[1] = static_cast<Real>(h.r); }
        else { hex_value[0] = static_cast<Real>(h.x); hex_value[1] = static_cast<Real>(h.y); }
        std::valarray<Real> const hx = _basis[std::slice(0, 2, 1)] * hex_value;
        std::valarray<Real> const hy = _basis[std::slice(2, 2, 1)] * hex_value;

        using Scalar = scalar_field_t<Point>;
        Point const p{ static_cast<Scalar>(std::round(hx.sum())),
                       static_cast<Scalar>(std::round(hy.sum())) };

        return Point{ p.x+static_cast<Scalar>(x),
                      p.y+static_cast<Scalar>(y) };
    }

    /**
     * Convert `p` to a point in hex space.
     *
     * `p` is meant to be in pixel space, and may to not correspond to an exact
     * hex point. As a result, `hex` will likely return a fractional hex that 
     * should be rounded to represent a meaningful hex coordinate. See
     * `hex_round` for a function that performs this rounding.
     */
    template<typename OutPoint, typename InPoint>
    OutPoint hex(const InPoint& p) const noexcept
    {
        using InField = scalar_field_t<InPoint>;
        using OutField = scalar_field_t<OutPoint>;

        InPoint const p2{ p.x-static_cast<InField>(x), p.y-static_cast<InField>(y) };
        std::valarray<Real> pv{ static_cast<OutField>(p2.x), static_cast<OutField>(p2.y) };

        auto q = _inverse[std::slice(0, 2, 1)] * pv;
        auto r = _inverse[std::slice(2, 2, 1)] * pv;

        return OutPoint{ q.sum(), r.sum() };
    }

    template<typename OutPoint, numeric InField>
    OutPoint hex(InField in_x, InField in_y)
    {
        using OutField = scalar_field_t<OutPoint>;
        std::valarray<OutField> hex_values
        {
            static_cast<OutField>(in_x)-x,
            static_cast<OutField>(in_y)-y
        };

        std::valarray<OutField> inverse_mul_q = _inverse[std::slice(0, 2, 1)];
        std::valarray<OutField> inverse_mul_r = _inverse[std::slice(2, 2, 1)];

        auto q = inverse_mul_q * hex_values;
        auto r = inverse_mul_r * hex_values;

        return OutPoint{ q.sum(), r.sum() };
    }

    /** Calculate the vertices of `hex` in screen space. */
    template<cartesian Point, typename Hex, std::output_iterator<Point> PointOutput>
    requires cartesian<Hex> or axial<Hex>
    PointOutput vertices(Hex const & h, PointOutput into_verts) const noexcept
    {
        auto center = pixel<Point>(h);
        Real constexpr pi = std::numbers::pi_v<Real>;
        Real const offset = _top_style == HexTop::Pointed? pi/6 : 0;

        // add each vertex to the list
        for (int i = 0; i < 6; ++i) {

            // calculate the angle of the vertex
            Real theta = offset + i * pi/3;

            // convert the angle to unit vector, then scale and offset
            std::valarray<Real> v{std::cos(theta), std::sin(theta)};
            v *= _unit_size;
            v += std::valarray<Real>{ static_cast<Real>(center.x),
                                   static_cast<Real>(center.y) };

            using Scalar = scalar_field_t<Point>;
            *into_verts++ = Point{ static_cast<Scalar>(std::round(v[0])),
                                   static_cast<Scalar>(std::round(v[1])) };
        }
        return into_verts;
    }
private:
    std::valarray<Real> _basis;
    std::valarray<Real> _inverse;

    HexTop _top_style;
    Real x; Real y;
    Real _unit_size;
};

using fbasis = Basis<float>;
}
