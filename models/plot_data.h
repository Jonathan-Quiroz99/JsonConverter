#pragma once

#include <string>
#include <vector>

enum class PlotType
{
    Scatter,
    Surface,
    Heatmap,
    Scatter3D
};

struct Series
{
    std::string name;

    std::vector<double> x;

    std::vector<double> y;

    std::vector<double> z;

    std::vector<std::vector<double>> matrix;
};

struct PlotData
{
    PlotType plotType;

    std::string title;
    std::string subtitle;

    std::string xLabel;
    std::string yLabel;
    std::string zLabel;

    std::string units;

    // Scatter plots
    std::vector<Series> series;

    // Surface plots
    std::vector<std::vector<double>> matrix;

	// Animated surface plots (multiple frames)
    std::vector<std::vector<std::vector<double>>> frames;
};