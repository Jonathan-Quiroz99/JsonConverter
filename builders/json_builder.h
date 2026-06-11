#pragma once

#include <string>

#include "../models/plot_data.h"

class JsonBuilder
{
public:

    static std::string build(
        const PlotData& data);

private:

    static std::string buildSurface(
		const PlotData& data);

    static std::string buildScatter(
        const PlotData& data);

    static std::string buildStaticSurface(
        const PlotData& data);

    static std::string buildHeatmap(
        const PlotData& data);

	static std::string buildScatter3D(
		const PlotData& data);

    static void writeMatrix(
        std::ostringstream& json,
        const std::vector<std::vector<double>>& matrix);

	static std::string buildAnimatedSurface(
		const PlotData& data);
};