// graph_module.cpp

#include "graph_module.hpp"
#include "tokenizer.hpp"
#include "parser.hpp"
#include "evaluator.hpp"

#include <limits>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <iomanip>

GraphModule::GraphModule(int width, int height)
    : width(width), height(height)
{
    // Suppress SFML console warnings
    sf::err().rdbuf(nullptr);

    // Load font
    if (!font.loadFromFile("fonts/DejaVuSans.ttf"))
        font.loadFromFile("../fonts/DejaVuSans.ttf");

    // Default viewport
    xmin = -10;
    xmax =  10;
    ymin = -10;
    ymax =  10;

    // Initial scaling
    xScale = float(width)  / float(xmax - xmin);
    yScale = float(height) / float(ymax - ymin);
}

void GraphModule::plot(const std::string& expr,
                       double cxmin, double cxmax,
                       int resolution)
{
    if (resolution <= 0)
        throw std::runtime_error("Resolution must be greater than 0");

    // Sample first so state stays consistent if it throws
    auto sampled = sample(expr, cxmin, cxmax, resolution);

    // Color palette
    static std::vector<sf::Color> palette = {
        sf::Color::Cyan,
        sf::Color::Red,
        sf::Color::Green,
        sf::Color::Yellow,
        sf::Color::Magenta,
        sf::Color::White
    };

    // Store metadata
    expressions.push_back(expr);
    curveXmin.push_back(cxmin);
    curveXmax.push_back(cxmax);
    resolutions.push_back(resolution);
    curveColors.push_back(palette[curveColors.size() % palette.size()]);

    // Store raw math-space points
    rawPoints.push_back(std::move(sampled));

    // Allocate corresponding screen-space curve
    curves.emplace_back();
}

void GraphModule::clear() {
    expressions.clear();
    resolutions.clear();
    curveColors.clear();
    curveXmin.clear();
    curveXmax.clear();
    rawPoints.clear();
    curves.clear();
}

std::vector<std::pair<double,double>>
GraphModule::sample(const std::string& expr,
                    double cxmin, double cxmax,
                    int resolution)
{
    if (resolution <= 0)
        throw std::runtime_error("Resolution must be greater than 0");

    // Parse expression
    Tokenizer tokenizer(expr);
    Parser parser(std::move(tokenizer));
    std::unique_ptr<ASTNode> ast = parser.parse();

    Evaluator eval;

    std::vector<std::pair<double,double>> pts;
    pts.reserve(resolution * 2);

    double baseStep = (cxmax - cxmin) / double(resolution);
    double step     = baseStep;

    double steepThreshold = baseStep * 2.0;
    double flatThreshold  = baseStep * 0.1;

    double minStep = baseStep / 25.0;

    double x = cxmin;
    double prevX = cxmin;
    double prevY = std::numeric_limits<double>::quiet_NaN();

    // Adaptive sampling loop in math-space
    while (x <= cxmax) {
        eval.variables["X"] = x;
        double y = eval.eval(ast.get());

        if (std::isfinite(y) && std::abs(y) < 1e6)
            pts.emplace_back(x, y);

        double nextX = x + step;
        if (nextX > cxmax) break;

        eval.variables["X"] = nextX;
        double nextY = eval.eval(ast.get());

        if (!std::isfinite(nextY) || std::abs(nextY - y) > 1e6) {
            nextY = y;
            step  = baseStep;
        }

        double slope1 = std::isfinite(prevY) ? (y - prevY) / (x - prevX) : 0.0;
        double slope2 = (nextY - y) / (nextX - x);
        double curvature = std::abs(slope2 - slope1);

        double targetStep = step;

        if (curvature > 0.5 || std::abs(nextY - y) > steepThreshold)
            targetStep *= 0.6;
        else if (curvature < 0.05 && std::abs(nextY - y) < flatThreshold)
            targetStep *= 1.2;

        double pixelStep = (cxmax - cxmin) / double(width);
        double maxAllowedStep = pixelStep * 0.75;

        targetStep = std::clamp(targetStep, minStep, maxAllowedStep);
        step = (step * 0.5) + (targetStep * 0.5);

        prevX = x;
        prevY = y;
        x += step;
    }

    return pts;
}

sf::Vector2f GraphModule::toScreen(double x, double y) {
    float sx = float((x - xmin) * xScale);
    float sy = float((ymax - y) * yScale);
    return {sx, sy};
}

void GraphModule::render() {
    // Update scaling for current viewport
    xScale = float(width)  / float(xmax - xmin);
    yScale = float(height) / float(ymax - ymin);

    sf::RenderWindow window(sf::VideoMode(width, height), "Graph");

    // UI buttons
    sf::RectangleShape zoomIn({80, 40});
    zoomIn.setPosition(10, 10);
    zoomIn.setFillColor(sf::Color(80, 80, 80));

    sf::RectangleShape zoomOut({80, 40});
    zoomOut.setPosition(10, 60);
    zoomOut.setFillColor(sf::Color(80, 80, 80));

    sf::RectangleShape clearBtn({80, 40});
    clearBtn.setPosition(10, 110);
    clearBtn.setFillColor(sf::Color(120, 40, 40));

    sf::RectangleShape saveBtn({80, 40});
    saveBtn.setPosition(10, 160);
    saveBtn.setFillColor(sf::Color(40, 120, 40));

    sf::Text tPlus("+", font, 24);  tPlus.setPosition(45, 15);
    sf::Text tMinus("-", font, 24); tMinus.setPosition(45, 65);
    sf::Text tClear("Clear", font, 20); tClear.setPosition(20, 115);
    sf::Text tSave("Save", font, 20);  tSave.setPosition(25, 165);

    sf::Text hover("", font, 16);
    hover.setFillColor(sf::Color::White);
    hover.setPosition(10, window.getSize().y - 30);

    bool dragging = false;
    sf::Vector2i lastMouse;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {

            // Close window
            if (event.type == sf::Event::Closed ||
                (event.type == sf::Event::KeyPressed &&
                 event.key.code == sf::Keyboard::Escape))
                window.close();

            // Reset viewport
            if (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::R) {
                xmin = -10; xmax = 10;
                ymin = -10; ymax = 10;
                xScale = float(width)  / float(xmax - xmin);
                yScale = float(height) / float(ymax - ymin);
            }

            // Zoom with mouse wheel
            if (event.type == sf::Event::MouseWheelScrolled) {
                sf::Vector2i m = sf::Mouse::getPosition(window);
                double cx = xmin + m.x / xScale;
                double cy = ymax - m.y / yScale;

                zoomFactor *= (event.mouseWheelScroll.delta > 0 ? 0.9 : 1.1);
                zoomFactor = std::clamp(zoomFactor, 0.01, 100.0);

                double xr = (xmax - xmin) * zoomFactor;
                double yr = (ymax - ymin) * zoomFactor;

                xmin = cx - xr / 2.0;
                xmax = cx + xr / 2.0;
                ymin = cy - yr / 2.0;
                ymax = cy + yr / 2.0;

                xScale = float(width)  / float(xmax - xmin);
                yScale = float(height) / float(ymax - ymin);
            }

            // Hover coordinate display
            if (event.type == sf::Event::MouseMoved && !dragging) {
                double x = xmin + event.mouseMove.x / xScale;
                double y = ymax - event.mouseMove.y / yScale;
                hover.setString("x=" + std::to_string(x) + "  y=" + std::to_string(y));
            }

            // Start dragging
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Middle) {
                dragging = true;
                lastMouse = sf::Mouse::getPosition(window);
            }

            // Stop dragging
            if (event.type == sf::Event::MouseButtonReleased &&
                event.mouseButton.button == sf::Mouse::Middle)
                dragging = false;

            // Pan viewport
            if (event.type == sf::Event::MouseMoved && dragging) {
                sf::Vector2i c = sf::Mouse::getPosition(window);
                double dx = (c.x - lastMouse.x) / xScale;
                double dy = (c.y - lastMouse.y) / yScale;

                xmin -= dx; xmax -= dx;
                ymin += dy; ymax += dy;

                lastMouse = c;

                xScale = float(width)  / float(xmax - xmin);
                yScale = float(height) / float(ymax - ymin);
            }

            // Left-click buttons
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left) {

                sf::Vector2f m(event.mouseButton.x, event.mouseButton.y);

                bool zIn  = zoomIn.getGlobalBounds().contains(m);
                bool zOut = zoomOut.getGlobalBounds().contains(m);
                bool clr  = clearBtn.getGlobalBounds().contains(m);
                bool sav  = saveBtn.getGlobalBounds().contains(m);

                if (clr) {
                    clear();
                    continue;
                }

                if (sav) {
                    sf::Texture tex;
                    tex.create(window.getSize().x, window.getSize().y);
                    tex.update(window);

                    namespace fs = std::filesystem;
                    std::string name;
                    std::cout << "Enter a name for the graph PNG: ";
                    std::getline(std::cin, name);

                    fs::path dir = "graphs";
                    if (!fs::exists(dir)) fs::create_directory(dir);

                    fs::path file = dir / (name + ".png");
                    tex.copyToImage().saveToFile(file.string());
                    std::cout << "Saved to: " << file << std::endl;
                    continue;
                }

                if (zIn || zOut) {
                    sf::Vector2i mp = sf::Mouse::getPosition(window);
                    double cx = xmin + mp.x / xScale;
                    double cy = ymax - mp.y / yScale;

                    zoomFactor *= (zIn ? 0.95 : 1.05);
                    zoomFactor = std::clamp(zoomFactor, 0.01, 100.0);

                    double xr = (xmax - xmin) * zoomFactor;
                    double yr = (ymax - ymin) * zoomFactor;

                    xmin = cx - xr / 2.0;
                    xmax = cx + xr / 2.0;
                    ymin = cy - yr / 2.0;
                    ymax = cy + yr / 2.0;

                    xScale = float(width)  / float(xmax - xmin);
                    yScale = float(height) / float(ymax - ymin);
                }
            }
        }

        // Update hover text
        sf::Vector2i mp = sf::Mouse::getPosition(window);
        double hx = xmin + mp.x / xScale;
        double hy = ymax - mp.y / yScale;
        hover.setString("x=" + std::to_string(hx) + "  y=" + std::to_string(hy));

        // Clear screen
        window.clear(sf::Color::Black);

        // Recompute screen-space points from raw math-space points
        for (size_t i = 0; i < rawPoints.size(); ++i) {
            curves[i].clear();
            curves[i].reserve(rawPoints[i].size());

            for (auto& p : rawPoints[i]) {
                curves[i].emplace_back(toScreen(p.first, p.second), curveColors[i]);
            }

            sf::VertexArray curve(sf::LineStrip, curves[i].size());
            for (size_t j = 0; j < curves[i].size(); ++j)
                curve[j] = curves[i][j];

            window.draw(curve);
        }

        // Draw grid
        sf::VertexArray grid(sf::Lines);
        double xStep = (xmax - xmin) / 20.0;
        double yStep = (ymax - ymin) / 20.0;

        for (double x = xmin; x <= xmax; x += xStep) {
            grid.append(sf::Vertex(toScreen(x, ymin), sf::Color(50, 50, 50)));
            grid.append(sf::Vertex(toScreen(x, ymax), sf::Color(50, 50, 50)));
        }

        for (double y = ymin; y <= ymax; y += yStep) {
            grid.append(sf::Vertex(toScreen(xmin, y), sf::Color(50, 50, 50)));
            grid.append(sf::Vertex(toScreen(xmax, y), sf::Color(50, 50, 50)));
        }

        window.draw(grid);

        // Draw axes
        sf::VertexArray axes(sf::Lines);

        if (xmin <= 0 && xmax >= 0) {
            axes.append(sf::Vertex(toScreen(0, ymin), sf::Color::White));
            axes.append(sf::Vertex(toScreen(0, ymax), sf::Color::White));
        }

        if (ymin <= 0 && ymax >= 0) {
            axes.append(sf::Vertex(toScreen(xmin, 0), sf::Color::White));
            axes.append(sf::Vertex(toScreen(xmax, 0), sf::Color::White));
        }

        window.draw(axes);

        // Draw axis labels
        for (double x = xmin; x <= xmax; x += xStep) {
            sf::Vector2f pos = toScreen(x, 0);
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << x;
            sf::Text label(oss.str(), font, 14);
            label.setFillColor(sf::Color::White);
            label.setPosition(pos.x - 5, pos.y + 5);
            window.draw(label);
        }

        for (double y = ymin; y <= ymax; y += yStep) {
            sf::Vector2f pos = toScreen(0, y);
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << y;
            sf::Text label(oss.str(), font, 14);
            label.setFillColor(sf::Color::White);
            label.setPosition(pos.x + 5, pos.y - 7);
            window.draw(label);
        }

        // Draw legend
        if (!expressions.empty()) {
            float h = 30.f * expressions.size() + 10.f;
            sf::RectangleShape box({200.f, h});
            box.setPosition(window.getSize().x - 210.f, 10.f);
            box.setFillColor(sf::Color(20, 20, 20, 200));
            box.setOutlineColor(sf::Color::White);
            box.setOutlineThickness(2.f);
            window.draw(box);

            for (size_t i = 0; i < expressions.size(); ++i) {
                sf::Text label(expressions[i], font, 16);
                label.setFillColor(curveColors[i]);
                label.setPosition(window.getSize().x - 200.f, 20.f + i * 30.f);
                window.draw(label);
            }
        }

        // Draw UI
        window.draw(zoomIn);
        window.draw(zoomOut);
        window.draw(clearBtn);
        window.draw(saveBtn);
        window.draw(tPlus);
        window.draw(tMinus);
        window.draw(tClear);
        window.draw(tSave);
        window.draw(hover);

        window.display();
    }
}
