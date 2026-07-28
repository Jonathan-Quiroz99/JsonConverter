#pragma once

#include <sstream>
#include <string>

#include "../../models/plot_data.h"

class LayoutWriter
{
public:

    static void writeTitle(
        std::ostringstream& json,
        const std::string& title);

    static void writeAxis(
        std::ostringstream& json,
        const std::string& axisName,
        const std::string& title);

    static void writeScene(
        std::ostringstream& json,
        const PlotData& data);

    static void write2DLayout(
        std::ostringstream& json,
        const PlotData& data);

    static void write3DLayout(
        std::ostringstream& json,
        const PlotData& data);
};