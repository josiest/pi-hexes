#include "pi/hexes.hpp"
#include <pi/geometry/affine_transforms.hpp>

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"

#include <vector>
#include <array>
#include <string>

namespace simple
{
struct window_settings
{
    std::string name = "Simple Hexagon Grid Example";
    sf::Vector2u dimensions{ 800u, 600u };
    std::uint32_t style = sf::Style::Titlebar | sf::Style::Close;
};

struct example_settings
{
    float pixels_per_unit = 40.f;
};

// convert a hex coordinate to an sfml shape
static sf::ConvexShape hex_shape(pi::HexTop hex_style,
                                 const pi::transform2f & pixel_from_world,
                                 const sf::Vector2f & p)
{
    sf::ConvexShape shape{6};
    // if sf::Shape had begin() -> output_iterator<Vector2f>
    // basis.vertices(hex, shape.begin());

    // compute the vertices for the shape
    std::array<sf::Vector2f, 6> verts;
    hex_vertices<sf::Vector2f>(hex_style, verts.begin());

    for (int i = 0; i < verts.size(); i++)
    {
        const auto vert = pixel_from_world * (p + verts[i]);
        shape.setPoint(i, vert);
    }
    return shape;
}
}

int main()
{
    const simple::window_settings window_settings;
    static constexpr simple::example_settings example_settings;
    static constexpr auto hex_style = pi::HexTop::Flat;

    // Create the basis for the grid, centered in the middle of the screen
    const auto world_from_hex = pi::hex_basis2<float>(hex_style);

    pi::transform2f pixel_from_world;
    pixel_from_world.scale(example_settings.pixels_per_unit);
    pixel_from_world.translation(static_cast<float>(window_settings.dimensions.x)/2.f,
                                 static_cast<float>(window_settings.dimensions.y)/2.f);

    // initialize the hexes we're working with
    // and set some basic graphical settings
    std::vector<sf::Vector2f> hexes;
    pi::hex_range(sf::Vector2f(0.f, 0.f), 3, std::back_inserter(hexes));

    std::vector<sf::ConvexShape> shapes;
    shapes.reserve(hexes.size());
    for (const auto& hex : hexes)
    {
        auto p = world_from_hex * hex;
        auto& shape = shapes.emplace_back(simple::hex_shape(hex_style, pixel_from_world, p));
        shape.setOutlineColor(sf::Color::Black);
        shape.setOutlineThickness(1.0f);
    }

    // Create the window, but make sure it's not resizeable
    sf::RenderWindow window( sf::VideoMode(window_settings.dimensions),
                             window_settings.name, window_settings.style );

    while (window.isOpen())
    {
        // close if exit button pressed
        while (const auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        // draw the hexes
        window.clear();
        for (const auto & hex : shapes) {
            window.draw(hex);
        }
        window.display();
    }
    return EXIT_SUCCESS;
}
