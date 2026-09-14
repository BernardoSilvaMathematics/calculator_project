#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class GraphModule {
public:
    GraphModule(int width, int height);

    // Add a curve with its own domain and resolution
    void plot(const std::string& expr,
              double cxmin, double cxmax,
              int resolution);

    // Remove all curves
    void clear();

    // Check if graph is empty
    bool empty() const { return rawPoints.empty(); }

    // Open the graph window and render everything
    void render();

private:
    // Sample curve in math-space (no screen coords)
    std::vector<std::pair<double,double>> sample(const std::string& expr,
                                                 double cxmin, double cxmax,
                                                 int resolution);

    // Convert math coordinates to screen coordinates
    sf::Vector2f toScreen(double x, double y);

    // Window size
    int width;
    int height;

    // Viewport (pan/zoom)
    double xmin;
    double xmax;
    double ymin;
    double ymax;

    // Pixel scaling
    float xScale;
    float yScale;

    // Zoom multiplier
    double zoomFactor = 1.0;

    // Font for labels
    sf::Font font;

    // Curve metadata
    std::vector<std::string> expressions;
    std::vector<int> resolutions;
    std::vector<sf::Color> curveColors;
    std::vector<double> curveXmin;
    std::vector<double> curveXmax;

    // Raw math-space points for each curve
    std::vector<std::vector<std::pair<double,double>>> rawPoints;

    // Screen-space points for rendering
    std::vector<std::vector<sf::Vertex>> curves;
};
