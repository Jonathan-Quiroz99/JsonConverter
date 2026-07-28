#pragma once

#include <sstream>

#include "../../models/plot_data.h"

class AnimationFramesWriter
{
public:

    static void writeSurfaceFrames(
        std::ostringstream& json,
        const PlotData& data);

    static void writeScatter3DFrames(
        std::ostringstream& json,
        const PlotData& data);

private:

    static void writeMatrix(
        std::ostringstream& json,
        const std::vector<std::vector<double>>& matrix);
};