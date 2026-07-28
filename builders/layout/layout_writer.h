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

    static void writeAnimatedScene(
        std::ostringstream& json,
        const PlotData& data,
        double xMin,
        double xMax,
        double yMin,
        double yMax,
        double zMin,
        double zMax);

    static void writeAnimatedAxis(
        std::ostringstream& json,
        const std::string& axisName,
        const std::string& title,
        double min,
        double max);

    static void writeCamera(
        std::ostringstream& json);

    static void writeAspectRatio(
        std::ostringstream& json);

    static void writeMargin(
        std::ostringstream& json,
        int top,
        int bottom,
        int left,
        int right);

    static void write2DLayout(
        std::ostringstream& json,
		const PlotData& data);

    static void write3DLayout(
        std::ostringstream& json,
		const PlotData& data);
};