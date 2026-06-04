#include "StrainEnergyConformParser.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

StrainEnergyConformParser::StrainEnergyConformParser(
    const std::string& path)
    :
    filepath(path)
{
}

PlotData StrainEnergyConformParser::parse()
{
    PlotData data;

    data.plotType =
        PlotType::Scatter;

    data.title =
        "Ring Strain Energy During Conformance";

    data.xLabel =
        "Output Step";

    data.yLabel =
        "Strain Energy";

    data.units =
        "in-lb";

    Series raw;
    raw.name =
        "Strain Energy";

    Series filtered;
    filtered.name =
        "Filtered Strain Energy";

    std::ifstream file(filepath);

    if (!file.is_open())
    {
        throw std::runtime_error(
            "Cannot open file");
    }

    std::string line;

    //
    // Skip header
    //

    for (int i = 0; i < 5; i++)
    {
        std::getline(file, line);
    }

    int step = 0;

    //
    // Data
    //

    while (std::getline(file, line))
    {
        if (line.empty())
        {
            continue;
        }

        for (char& c : line)
        {
            if (c == ',')
            {
                c = ' ';
            }

            if (c == 'D')
            {
                c = 'E';
            }
        }

        std::stringstream ss(line);

        double rawValue;
        double filteredValue;

        if (!(ss >>
            rawValue >>
            filteredValue))
        {
            continue;
        }

        raw.x.push_back(step);
        raw.y.push_back(rawValue);

        filtered.x.push_back(step);
        filtered.y.push_back(filteredValue);

        step++;
    }

    data.series.push_back(raw);
    data.series.push_back(filtered);

    return data;
}