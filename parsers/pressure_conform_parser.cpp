#include "pressure_conform_parser.h"

#include <fstream>
#include <sstream>

PressureConformParser::PressureConformParser(
    const std::string& path)
    :
    filepath(path)
{
}

PlotData PressureConformParser::parse()
{
    PlotData data;

    data.plotType = PlotType::Surface;

    data.title =
        "Ring Bottom Side Pressure";

    data.xLabel =
        "Circumferential Node";

    data.yLabel =
        "Radial Side Node";

    data.zLabel =
        "Pressure";

    std::ifstream file(filepath);

    if (!file.is_open())
    {
        return data;
    }

    std::string line;

    // Header
    std::getline(file, line);
    std::getline(file, line);
    std::getline(file, line);
    std::getline(file, line);
    std::getline(file, line);

    while (std::getline(file, line))
    {
        if (line.empty())
        {
            continue;
        }

        std::vector<double> row;

        for (char& c : line)
        {
            if (c == ',')
                c = ' ';

            if (c == 'D')
                c = 'E';
        }

        std::stringstream ss(line);

        double value;

        while (ss >> value)
        {
            row.push_back(value);
        }

        if (!row.empty())
        {
            data.matrix.push_back(row);
        }
    }

    return data;
}