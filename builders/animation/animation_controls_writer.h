#pragma once

#include <sstream>
#include "../../models/plot_data.h"

class AnimationControlsWriter
{
public:

    static void writeButtons(
        std::ostringstream& json,
        int playDuration = 50);

    static void writeSlider(
        std::ostringstream& json,
        const PlotData& data,
        int topPadding,
        const std::string& prefix = "Crank Angle: ");
};