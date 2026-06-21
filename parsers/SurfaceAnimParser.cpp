#include "SurfaceAnimParser.h"

#include <fstream>
#include <sstream>
#include <iostream>

SurfaceAnimParser::SurfaceAnimParser(
    const std::string& path)
    :
    filepath(path)
{
}

PlotData SurfaceAnimParser::parse()
{
    PlotData data;

	data.plotType = PlotType::Surface;

    data.xLabel = "Circumferential Node";
    data.yLabel = "Axial Node";

    if (filepath.find("FACEPRES") != std::string::npos)
    {
        data.title = "Ring Face Pressure Cycle";
        data.zLabel = "Pressure";
        data.units = "PSI";
    }
    else if (filepath.find("FACE_STRESS") != std::string::npos)
    {
        data.title = "Face Von-Mises Stress Cycle";
        data.zLabel = "Von-Mises Stress";
        data.units = "PSI";
    }
    else if (filepath.find("TOP_STRESS") != std::string::npos)
    {
        data.title = "Top Von-Mises Stress Cycle";
        data.zLabel = "Von-Mises Stress";
        data.units = "PSI";
    }
    else if (filepath.find("TRA_CYCLE") != std::string::npos)
    {
        data.title = "Ring Top Side Pressure Cycle";
        data.zLabel = "Pressure";
        data.units = "PSI";
    }


    std::ifstream file(filepath);

    if (!file.is_open())
    {
        return data;
    }

    std::string line;

    constexpr int HEADER_LINES = 5;
    constexpr int ROWS_PER_FRAME = 101;
    constexpr int COLUMNS = 9;

    for (int i = 0; i < HEADER_LINES; i++)
    {
        std::getline(file, line);
    }

    while (true)
    {
        std::vector<std::vector<double>> frame;

        bool frameRead = false;

        for (int row = 0;
            row < ROWS_PER_FRAME;
            row++)
        {
            if (!std::getline(file, line))
            {
                break;
            }

            if (line.empty())
            {
                continue;
            }

            frameRead = true;

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

            std::vector<double> values;

            double value;

            while (ss >> value)
            {
                values.push_back(value);
            }

            if (values.size() == COLUMNS)
            {
                frame.push_back(values);
            }
        }

        if (!frameRead)
        {
            break;
        }

        data.frames.push_back(frame);
    }

    if (!data.frames.empty())
    {
        data.matrix =
            data.frames.front();
    }

    std::cout
        << "TOTAL FRAMES: "
        << data.frames.size()
        << std::endl;

    return data;
}