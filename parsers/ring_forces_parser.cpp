#include "ring_forces_parser.h"

#include <fstream>
#include <sstream>

RingForcesParser::RingForcesParser(
    const std::string& path)
    :
    filepath(path)
{
}

PlotData RingForcesParser::parse()
{
    PlotData data;

    data.plotType = PlotType::Scatter;

    data.title =
        "Ring Forces";

    data.xLabel =
        "Crank Angle";

    data.yLabel =
        "Force";

    data.units =
        "N";

    Series gas;
    gas.name = "Gas Pressure Force";

    Series friction;
    friction.name = "Friction Force";

    Series inertial;
    inertial.name = "Inertial Force";

    std::ifstream file(filepath);

    if (!file.is_open())
    {
        return data;
    }

    std::string line;

    std::getline(file, line);
    std::getline(file, line);
    std::getline(file, line);

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

        double angle;
        double gasValue;
        double frictionValue;
        double inertialValue;

        if (!(ss >>
            angle >>
            gasValue >>
            frictionValue >>
            inertialValue))
        {
            continue;
        }

        gas.x.push_back(angle);
        gas.y.push_back(gasValue);

        friction.x.push_back(angle);
        friction.y.push_back(frictionValue);

        inertial.x.push_back(angle);
        inertial.y.push_back(inertialValue);
    }

    data.series.push_back(gas);
    data.series.push_back(friction);
    data.series.push_back(inertial);

    return data;
}