#pragma once
#include <string>
#include <vector>

enum class PlotType
{
    UNKNOWN = 0,
    SCATTER,
    HEATMAP,
    SURFACE,
    MESH3D,
    CONTOUR,
    HISTOGRAM
};

enum class DataType
{
    UNKNOWN = 0,
    STRESS,
    PRESSURE,
    TEMPERATURE,
    WEAR,
    FORCE,
    DISPLACEMENT,

    STRAIN_ENERGY
};

struct ClassificationResult
{
    PlotType plot_type = PlotType::UNKNOWN;
    DataType data_type = DataType::UNKNOWN;
    double confidence = 0.0;
    std::vector<std::string> matched_rules;
    bool parser_available = false;
};