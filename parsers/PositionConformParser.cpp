#include "PositionConformParser.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

PositionConformParser::PositionConformParser(
    const std::string& path)
    :
    filepath(path)
{
}

PlotData PositionConformParser::parse()
{
    PlotData data;

    data.plotType =
        PlotType::Scatter3D;

    data.title =
        "Ring Position During Conformance";

    data.xLabel = "X";
    data.yLabel = "Y";
    data.zLabel = "Z";

    Series ring;
    ring.name = "Ring";

    std::ifstream file(filepath);

    if (!file.is_open())
    {
        throw std::runtime_error(
            "Cannot open file");
    }

    std::string line;

    for (int i = 0; i < 6; i++)
    {
        std::getline(file, line);
    }

    while (std::getline(file, line))
    {
        for (char& c : line)
        {
            if (c == ',')
                c = ' ';

            if (c == 'D')
                c = 'E';
        }

        std::stringstream ss(line);

        double x;
        double y;
        double z;
        double twist;

        if (!(ss >> x >> y >> z >> twist))
        {
            continue;
        }

        ring.x.push_back(x);
        ring.y.push_back(y);
        ring.z.push_back(z);
    }

    data.series.push_back(ring);

    return data;
}