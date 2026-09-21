#include "pi/hexes.hpp"
#include <SFML/Graphics.hpp>

#include <optional>
#include <unordered_set>
#include <vector>
#include <array>
#include <cstdint>

#include <pi/geometry.hpp>

namespace highlight
{
struct window_settings
{
    std::string name = "Hex Highlight Example";
    sf::Vector2u dimensions{ 800u, 600u };
    std::uint32_t style = sf::Style::Titlebar | sf::Style::Close;
};

struct world_settings
{
    float pixels_per_unit = 40.f;
};

struct ui_settings
{
    std::string font = "arial.ttf";
};
}

template<>
struct std::hash<sf::Vector2i>
{
    size_t operator()(const sf::Vector2i & h) const noexcept
    {
        constexpr hash<float> float_hash;
        const size_t hex_x = float_hash(static_cast<float>(h.x));
        const size_t hex_y = float_hash(static_cast<float>(h.y));
        return hex_x ^ (hex_y + 0x9e3779b9 + (hex_x << 6) + (hex_x >> 2));
    }
};

sf::Vector2f as_float(const sf::Vector2i & v)
{
    return { static_cast<float>(v.x), static_cast<float>(v.y) };
}

// convert a hex coordinate to sfml shape
sf::ConvexShape hex_shape(pi::HexTop hex_style, const pi::transform2f & pixel_from_world, const sf::Vector2f & p)
{
    std::array<sf::Vector2f, 6> verts;
    pi::hex_vertices<sf::Vector2f>(hex_style, verts.begin());

    sf::ConvexShape shape{6};
    for (int i = 0; i < verts.size(); i++)
    {
        shape.setPoint(i, pixel_from_world * (p + verts[i]));
    }
    return shape;
}

enum class Axis { X, Y };

class axis_line
{
public:
    axis_line() = default;
    axis_line(const pi::transform2f & basis, Axis axis);
    void draw(sf::RenderWindow & window);
private:
    static constexpr int axis_size = 20;
    static constexpr sf::Color x_color = sf::Color::Black;
    static constexpr sf::Color y_color = sf::Color::Black;
    std::array<sf::Vertex, 2> line;
};

axis_line::axis_line(const pi::transform2f & basis, Axis axis)
{
    if (axis == Axis::X)
    {
        line[0] = sf::Vertex(basis * sf::Vector2f(-axis_size, 0.f), x_color);
        line[1] = sf::Vertex(basis * sf::Vector2f(axis_size, 0.f), x_color);
    }
    else
    {
        line[0] = sf::Vertex(basis * sf::Vector2f(0.f, -axis_size), y_color);
        line[1] = sf::Vertex(basis * sf::Vector2f(0.f, axis_size), y_color);
    }
}

void axis_line::draw(sf::RenderWindow & window)
{
    window.draw(line.data(), line.size(), sf::PrimitiveType::Lines);
}

class highlight_system {
public:
    using HexCoord = sf::Vector2i;
    highlight_system(pi::HexTop hex_style, float width, float height, float unit_size);

    void on_mouse_move(int x, int y);
    void on_mouse_pressed(int x, int y);
    void on_mouse_released();
    void draw(sf::RenderWindow& window);

    pi::transform2f world_from_hex;
    pi::transform2f pixel_from_world;
    pi::HexTop hex_style;

    axis_line x_axis;
    axis_line y_axis;

    // all hex shapes to be drawn
    using ShapeEntry = std::pair<HexCoord, sf::ConvexShape>;
    std::vector<ShapeEntry> shapes;

    // hovered will keep track of which hex the mouse is currently over
    std::optional<HexCoord> hovered = std::nullopt;

    // clicked will keep track of which hex was clicked if mouse button is down
    std::optional<HexCoord> clicked = std::nullopt;

    // clicked_range will keep track of all the hexes from clicked to hovered
    std::unordered_set<HexCoord> clicked_range;
};

highlight_system::highlight_system(pi::HexTop hex_style, float width, float height, float unit_size)
    : hex_style(hex_style)
{
    world_from_hex = pi::hex_basis2<float>(hex_style);

    pixel_from_world.scale(unit_size);
    pixel_from_world.translation(width/2.f, height/2.f);
    pixel_from_world.parent = &world_from_hex;

    x_axis = axis_line(pixel_from_world, Axis::X);
    y_axis = axis_line(pixel_from_world, Axis::Y);

    // initialize the set of hexes we're working with
    // and set some basic graphical settings
    std::vector<HexCoord> hexes;
    pi::hex_range(HexCoord(0, 0), 30, std::back_inserter(hexes));

    for (const auto & hex : hexes)
    {
        const auto p = world_from_hex * as_float(hex);
        auto & [_, shape] = shapes.emplace_back(hex, hex_shape(hex_style, pixel_from_world.local(), p));
        shape.setOutlineColor(sf::Color::Black);
        shape.setOutlineThickness(1.0f);
    }
}

void highlight_system::on_mouse_move(int x, int y)
{
    // convert the sfml point to a pi point
    // and round it to the nearest hex
    const auto xf = static_cast<float>(x); const auto yf = static_cast<float>(y);
    hovered = pi::nearest_hex<HexCoord>(pixel_from_world.inverse(sf::Vector2f(xf, yf)));

    // if the mouse button is down, update the line from the
    // clicked hex to the hovered hex
    if (clicked)
    {
        clicked_range.clear();
        const auto into_clicked = std::inserter(clicked_range, clicked_range.begin());
        pi::line(*clicked, *hovered, into_clicked);
    }
}

void highlight_system::on_mouse_pressed(int x, int y)
{
    // keep track of the clicked coordinate when the mouse button gets pressed
    const auto xf = static_cast<float>(x); const auto yf = static_cast<float>(y);
    clicked = pi::nearest_hex<HexCoord>(pixel_from_world.inverse(sf::Vector2f(xf, yf)));
}

void highlight_system::on_mouse_released()
{
    // reset clicked data when the mouse button is released
    clicked = std::nullopt;
    clicked_range.clear();
}

void highlight_system::draw(sf::RenderWindow & window)
{
    for (auto & [hex_coord, shape] : shapes)
    {
        // color selected tiles cyan and non selected tiles white
        shape.setFillColor(hex_coord == hovered or clicked_range.contains(hex_coord)?
                           sf::Color::Cyan : sf::Color::White);
        window.draw(shape);
    }
    x_axis.draw(window);
    y_axis.draw(window);
}

int main()
{
    // Create the window, but make sure it's not resizeable
    const highlight::window_settings window_settings;
    const highlight::ui_settings ui_settings;
    constexpr highlight::world_settings world_settings;

    sf::RenderWindow window{ sf::VideoMode(window_settings.dimensions),
                             window_settings.name, window_settings.style };

    // Create the basis for the grid - centered in the middle of the screen
    highlight_system system(pi::HexTop::Pointed,
                            static_cast<float>(window_settings.dimensions.x),
                            static_cast<float>(window_settings.dimensions.y),
                            world_settings.pixels_per_unit);

    sf::Font font(ui_settings.font);
    std::vector<sf::Text> coordinate_labels; coordinate_labels.reserve(system.shapes.size());
    for (const auto & hex : system.shapes | std::views::keys)
    {
        auto & text = coordinate_labels.emplace_back(font);
        text.setString(std::format("{}, {}", hex.x, hex.y));
        text.setCharacterSize(12);
        text.setFillColor(sf::Color::Black);
        text.setPosition(system.pixel_from_world * as_float(hex));
    }
    static constexpr auto line_color = sf::Color::Black;
    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            if (const auto * mouse_moved = event->getIf<sf::Event::MouseMoved>())
            {
                system.on_mouse_move(mouse_moved->position.x, mouse_moved->position.y);
            }
            if (const auto * mouse_pressed = event->getIf<sf::Event::MouseButtonPressed>())
            {
                if (mouse_pressed->button == sf::Mouse::Button::Left)
                {
                    system.on_mouse_pressed(mouse_pressed->position.x, mouse_pressed->position.y);
                }
            }
            if (const auto * mouse_released = event->getIf<sf::Event::MouseButtonReleased>())
            {
                if (mouse_released->button == sf::Mouse::Button::Left)
                {
                    system.on_mouse_released();
                }
            }
        }
        window.clear();
        system.draw(window);
        for (const auto & coordinate_label : coordinate_labels)
        {
            window.draw(coordinate_label);
        }
        {
            const auto mouse = sf::Mouse::getPosition(window);
            auto y_projection = system.pixel_from_world.inverse(as_float(mouse));
            y_projection.x = 0;

            std::array mouse_x_line
            {
                sf::Vertex(system.pixel_from_world * y_projection, line_color),
                sf::Vertex(as_float(mouse), line_color),
            };
            window.draw(mouse_x_line.begin(), mouse_x_line.size(), sf::PrimitiveType::Lines);

            auto x_projection = system.pixel_from_world.inverse(as_float(mouse));
            x_projection.y = 0;
            std::array mouse_y_line
            {
                sf::Vertex(system.pixel_from_world * x_projection, line_color),
                sf::Vertex(as_float(mouse), line_color),
            };
            window.draw(mouse_y_line.begin(), mouse_y_line.size(), sf::PrimitiveType::Lines);
        }
        window.display();
    }
}