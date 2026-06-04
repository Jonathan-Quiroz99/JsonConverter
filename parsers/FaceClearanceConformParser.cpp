#include "FaceClearanceConformParser.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

FaceClearanceConformParser::FaceClearanceConformParser(
    const std::string& path)
    :
    filepath(path)
{
}

PlotData FaceClearanceConformParser::parse()
{
    PlotData data;

    data.plotType = PlotType::Surface;

    data.title =
        "Ring Face Clearance During Conformance";

    data.xLabel =
        "Circumferential Node";

    data.yLabel =
        "Axial Node";

    data.zLabel =
        "Clearance (in)";

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

    //
    // Matrix data
    //

    while (std::getline(file, line))
    {
        if (line.empty())
        {
            continue;
        }

        std::stringstream ss(line);

        std::vector<double> row;

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