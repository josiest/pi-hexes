#include "tess/tess.hpp"
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"

#include <vector>
#include <array>
#include <string>

#include <iostream>

namespace simple
{
struct window_settings
{
    std::string name = "Simple Tess Example";
    sf::Vector2u dimensions{ 800u, 600u };
    std::uint32_t style = sf::Style::Titlebar | sf::Style::Close;
};

struct example_settings
{
    float unit_size = 40.f;
};

// convert a hex coordinate to an sfml shape
template<typename Point> requires tess::axial<Point> or tess::cartesian<Point>
static sf::ConvexShape hex_shape(const tess::flat_fbasis& basis, const Point & hex)
{
    sf::ConvexShape shape{6};
    // if sf::Shape had begin() -> output_iterator<Vector2f>
    // basis.vertices(hex, shape.begin());

    // compute the vertices for the shape
    std::array<sf::Vector2f, 6> verts;
    basis.vertices<sf::Vector2f>(hex, verts.begin());
    for (int i = 0; i < verts.size(); i++)
    {
        shape.setPoint(i, verts[i]);
    }
    return shape;
}
}

int main()
{
    const simple::window_settings window_settings;
    static constexpr simple::example_settings example_settings;

    // Create the basis for the grid, centered in the middle of the screen
    const tess::flat_fbasis basis
    {
        static_cast<float>(window_settings.dimensions.x)/2.f,
        static_cast<float>(window_settings.dimensions.y)/2.f,
        example_settings.unit_size
    };

    // initialize the hexes we're working with
    // and set some basic graphical settings
    std::vector<sf::Vector2i> hexes;
    tess::hex_range(sf::Vector2i(0, 0), 3, std::back_inserter(hexes));

    std::vector<sf::ConvexShape> shapes;
    shapes.reserve(hexes.size());
    for (const auto& hex : hexes)
    {
        auto& shape = shapes.emplace_back(simple::hex_shape(basis, hex));
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
        for (const auto& hex : shapes) {
            window.draw(hex);
        }
        window.display();
    }
    return EXIT_SUCCESS;
}
