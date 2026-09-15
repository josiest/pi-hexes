#include "tess/tess.hpp"
#include <SFML/Graphics.hpp>

#include <optional>
#include <unordered_set>
#include <vector>
#include <array>
#include <cstdint>

namespace highlight
{
struct window_settings
{
    std::string name = "Tess Highlight Example";
    sf::Vector2u dimensions{ 800u, 600u };
    std::uint32_t style = sf::Style::Titlebar | sf::Style::Close;
};

struct world_settings
{
    float unit_size = 30.f;
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

// convert a hex coordinate to sfml shape
template<typename Hex> requires tess::axial<Hex> or tess::cartesian<Hex>
sf::ConvexShape hex_shape(const tess::fbasis& basis, const Hex& hex)
{
    std::array<sf::Vector2f, 6> verts;
    basis.vertices<sf::Vector2f>(hex, verts.begin());

    sf::ConvexShape shape{6};
    for (int i = 0; i < verts.size(); i++) {
        shape.setPoint(i, verts[i]);
    }
    return shape;
}

class highlight_system {
public:
    using HexCoord = sf::Vector2i;

    explicit highlight_system(const tess::fbasis& basis);
    void on_mouse_move(int x, int y);
    void on_mouse_pressed(int x, int y);
    void on_mouse_released();
    void draw(sf::RenderWindow& window);

    tess::fbasis basis;

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

highlight_system::highlight_system(const tess::fbasis& basis)
    : basis{ basis }
{
    // initialize the set of hexes we're working with
    // and set some basic graphical settings
    std::vector<HexCoord> hexes;
    tess::hex_range(HexCoord(0, 0), 30, std::back_inserter(hexes));

    for (const auto & hex : hexes)
    {
        auto & [_, shape] = shapes.emplace_back(hex, hex_shape(basis, hex));
        shape.setOutlineColor(sf::Color::Black);
        shape.setOutlineThickness(1.0f);
    }
}

void highlight_system::on_mouse_move(int x, int y)
{
    // convert the sfml point to a tess point
    // and round it to the nearest hex
    hovered = tess::nearest_hex<HexCoord>(basis.hex<sf::Vector2f>(x, y));

    // if the mouse button is down, update the line from the
    // clicked hex to the hovered hex
    if (clicked)
    {
        clicked_range.clear();
        const auto into_clicked = std::inserter(clicked_range, clicked_range.begin());
        tess::line(*clicked, *hovered, into_clicked);
    }
}

void highlight_system::on_mouse_pressed(int x, int y)
{
    // keep track of the clicked coordinate when the mouse button gets pressed
    clicked = tess::nearest_hex<HexCoord>(basis.hex<sf::Vector2f>(x, y));
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
}

int main()
{
    // Create the window, but make sure it's not resizeable
    const highlight::window_settings window_settings;
    constexpr highlight::world_settings world_settings;

    sf::RenderWindow window{ sf::VideoMode(window_settings.dimensions),
                             window_settings.name, window_settings.style };

    // Create the basis for the grid - centered in the middle of the screen
    const tess::fbasis basis
    {
        static_cast<float>(window_settings.dimensions.x)/2.f,
        static_cast<float>(window_settings.dimensions.y)/2.f,
        world_settings.unit_size,
        tess::HexTop::Pointed
    };
    highlight_system system(basis);

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
        window.display();
    }
}