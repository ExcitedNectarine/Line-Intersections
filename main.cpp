#include <cmath>
#include <array>
#include <SFML/Graphics.hpp>

static const double PI = 3.14159265359;
static const double DEGREE_RADIAN_RATIO = 0.0174533;

double angleBetweenPoints(const sf::Vector2f& a, const sf::Vector2f& b)
{
    return std::atan2(a.y - b.y, a.x - b.x) * 180.0000 / PI;
}

bool lineLineIntersection(const std::array<sf::Vertex, 2>& line1, const std::array<sf::Vertex, 2>& line2, sf::Vector2f* intersect = nullptr)
{
    // Get the points of the lines.
    auto p1 = line1[0].position, p2 = line1[1].position;
    auto p3 = line2[0].position, p4 = line2[1].position;

    // X and Y differences for both lines.
    float dx1 = p1.x - p2.x;
    float dy1 = p1.y - p2.y;
    float dx2 = p3.x - p4.x;
    float dy2 = p3.y - p4.y;

    // Find the denominator for the equation to find the intersect point.
    double denominator = (dx1 * dy2) - (dy1 * dx2);

    // This tests if the two lines are parallel or colinear.
    if (static_cast<int>(std::round(denominator)) == 0)
    {
        return false;
    }

    float a = (p1.x * p2.y) - (p1.y * p2.x);
    float b = (p3.x * p4.y) - (p3.y * p4.x);

    float x_numerator = (a * dx2) - (b * dx1);
    float y_numerator = (a * dy2) - (b * dy1);

    double x_intersect = x_numerator / denominator;
    double y_intersect = y_numerator / denominator;

    double rx1 = (x_intersect - p1.x) / -dx1;
    double ry1 = (y_intersect - p1.y) / -dy1;
    double rx2 = (x_intersect - p3.x) / -dx2;
    double ry2 = (y_intersect - p3.y) / -dy2;

    if (((rx1 >= 0 && rx1 <= 1) || (ry1 >= 0 && ry1 <= 1)) &&
        ((rx2 >= 0 && rx2 <= 1) || (ry2 >= 0 && ry2 <= 1)))
    {
        if (intersect)
        {
            intersect->x = static_cast<float>(x_intersect);
            intersect->y = static_cast<float>(y_intersect);
        }
        return true;
    }

    return false;
}

std::array<sf::Vector2f, 4> getRectVertices(const sf::FloatRect& rect)
{
    sf::Vector2f p1(rect.left, rect.top);
    sf::Vector2f p2(rect.left + rect.width, rect.top);
    sf::Vector2f p3(rect.left + rect.width, rect.top + rect.height);
    sf::Vector2f p4(rect.left, rect.top + rect.height);

    return { p1, p2, p3, p4 };
}

std::array<std::array<sf::Vertex, 2>, 4> getRectLines(const sf::FloatRect& rect)
{
    auto vertices = getRectVertices(rect);

    std::array<sf::Vertex, 2> line1, line2, line3, line4;
    line1[0].position = vertices[0]; line1[1].position = vertices[1];
    line2[0].position = vertices[1]; line2[1].position = vertices[2];
    line3[0].position = vertices[2]; line3[1].position = vertices[3];
    line4[0].position = vertices[3]; line4[1].position = vertices[0];

    return { line1, line2, line3, line4 };
}

sf::FloatRect getLineRect(const std::array<sf::Vertex, 2>& line)
{
    return {
        std::min(line[0].position.x, line[1].position.x),
        std::min(line[0].position.y, line[1].position.y),
        std::fabs(line[0].position.x - line[1].position.x),
        std::fabs(line[0].position.y - line[1].position.y)
    };
}

bool lineRectIntersection(const std::array<sf::Vertex, 2>& line, const sf::FloatRect& rect, sf::Vector2f* intersect = nullptr)
{
    auto line_rect = getLineRect(line);
    if (rect.intersects(line_rect))
    {
        auto lines = getRectLines(rect);
        return std::any_of(lines.begin(), lines.end(), [=](const std::array<sf::Vertex, 2>& rect_line) { return lineLineIntersection(line, rect_line, intersect); });
    }
    return false;
}

std::array<sf::Vertex, 2> castRay(const std::vector<sf::FloatRect>& rects, const std::vector<std::array<sf::Vertex, 2>>& lines, const sf::Vector2f& start, const double angle, const double limit)
{
    sf::Vector2f size;

    std::array<sf::Vertex, 2> ray =
    {
        sf::Vertex(start),
        sf::Vertex(start)
    };

    float x = static_cast<float>(std::cos(angle * DEGREE_RADIAN_RATIO));
    float y = static_cast<float>(std::sin(angle * DEGREE_RADIAN_RATIO));

    bool hit = false;
    while (std::sqrt((size.x * size.x) + (size.y * size.y)) <= limit && !hit)
    {
        ray[1].position.x += x;
        ray[1].position.y += y;
        size = ray[0].position - ray[1].position;

        hit = std::any_of(
            rects.begin(),
            rects.end(),
            [=](const sf::FloatRect& rect) { return lineRectIntersection(ray, rect, const_cast<sf::Vector2f*>(&ray[1].position)); }
        ) || std::any_of(
            lines.begin(),
            lines.end(),
            [=](const std::array<sf::Vertex, 2>& line) { return lineLineIntersection(ray, line, const_cast<sf::Vector2f*>(&ray[1].position)); }
        );
    }

    return ray;
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Line Intersections");
    window.setFramerateLimit(60);

    sf::Vector2f mouse_position;
    double angle;

    std::array<sf::Vertex, 2> line =
    {
        sf::Vertex(static_cast<sf::Vector2f>(window.getSize()) / 2.0f),
        sf::Vertex(static_cast<sf::Vector2f>(window.getSize()) / 2.0f)
    };

    sf::FloatRect line_rect = getLineRect(line);
    sf::RectangleShape line_rect_draw;
    line_rect_draw.setOutlineColor(sf::Color::Green);
    line_rect_draw.setOutlineThickness(1);
    line_rect_draw.setFillColor(sf::Color::Transparent);

    std::array<sf::Vertex, 2> line1 =
    {
        sf::Vertex(sf::Vector2f(30, 30)),
        sf::Vertex(sf::Vector2f(30, 400))
    };

    sf::RectangleShape rect;
    rect.setPosition(600, 100);
    rect.setSize(sf::Vector2f(100, 50));
    rect.setFillColor(sf::Color::Blue);

    sf::RectangleShape rect2;
    rect2.setPosition(100, 400);
    rect2.setSize(sf::Vector2f(100, 100));
    rect2.setFillColor(sf::Color::Blue);

    std::vector<std::array<sf::Vertex, 2>> lines;
    lines.push_back(line1);

    std::vector<sf::FloatRect> rects;
    rects.push_back(rect.getGlobalBounds());
    rects.push_back(rect2.getGlobalBounds());

    sf::Event event;
    while (window.isOpen())
    {
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            else if (event.type == sf::Event::MouseMoved)
            {
                mouse_position = window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
                angle = angleBetweenPoints(mouse_position, line[0].position);
                line = castRay(rects, lines, line[0].position, angle, 500);
                line_rect = getLineRect(line);
                line_rect_draw.setPosition(line_rect.left, line_rect.top);
                line_rect_draw.setSize(sf::Vector2f(line_rect.width, line_rect.height));
            }
        }

        window.clear();
        window.draw(rect);
        window.draw(rect2);
        window.draw(line_rect_draw);
        window.draw(line.data(), 2, sf::Lines);
        window.draw(line1.data(), 2, sf::Lines);
        window.display();
    }

    return 0;
}