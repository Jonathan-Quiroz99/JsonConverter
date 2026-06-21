#include "Scatter3dAnimParser.h"

#include <fstream>
#include <sstream>
#include <iostream>

Scatter3dAnimParser::Scatter3dAnimParser(
    const std::string& path)
    :
    filepath(path)
{
}

PlotData Scatter3dAnimParser::parse()
{
    PlotData data;

    data.plotType = PlotType::Scatter3D;

    data.title =
        "Ring Position Cycle";

    data.xLabel =
        "X Position";

    data.yLabel =
        "Y Position";

    data.zLabel =
        "Z Position";

    data.units =
        "Inches";

    std::ifstream file(filepath);

    if (!file.is_open())
    {
        return data;
    }

    std::string line;

    constexpr int HEADER_LINES = 5;

    /*
    ============================================================
    POS_CYCLE

    NODES = 9

    Each frame contains:

        X
        Y
        Z
        TWIST

    for 9 nodes.

    ============================================================
    */

    constexpr int ROWS_PER_FRAME = 9;
    constexpr int COLUMNS = 4;

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
                std::cout
                    << "WARNING: Empty line found in frame "
                    << data.frames.size()
                    << " row "
                    << row
                    << std::endl;

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

            if (values.size() != COLUMNS)
            {
                std::cout
                    << "BAD ROW in frame "
                    << data.frames.size()
                    << " row "
                    << row
                    << " values="
                    << values.size()
                    << std::endl;

                std::cout
                    << "LINE: "
                    << line
                    << std::endl;

                continue;
            }

            frame.push_back(values);
        }

        if (!frameRead)
        {
            break;
        }

        /*
        ============================================================
        FRAME VALIDATION
        ============================================================
        */

        if (frame.size() != ROWS_PER_FRAME)
        {
            std::cout
                << "BAD FRAME "
                << data.frames.size()
                << " size="
                << frame.size()
                << std::endl;

            break;
        }

        double minX = frame[0][0];
        double maxX = frame[0][0];

        double minY = frame[0][1];
        double maxY = frame[0][1];

        double minZ = frame[0][2];
        double maxZ = frame[0][2];

        for (const auto& node : frame)
        {
            minX = std::min(minX, node[0]);
            maxX = std::max(maxX, node[0]);

            minY = std::min(minY, node[1]);
            maxY = std::max(maxY, node[1]);

            minZ = std::min(minZ, node[2]);
            maxZ = std::max(maxZ, node[2]);
        }

        if (std::abs(maxX) > 100.0 ||
            std::abs(minX) > 100.0 ||
            std::abs(maxY) > 100.0 ||
            std::abs(minY) > 100.0 ||
            std::abs(maxZ) > 100.0 ||
            std::abs(minZ) > 100.0)
        {
            std::cout
                << "SUSPICIOUS FRAME "
                << data.frames.size()
                << std::endl;
        }

        for (size_t node = 0; node < frame.size(); node++)
        {
            if (std::abs(frame[node][0]) > 10.0 ||
                std::abs(frame[node][1]) > 10.0 ||
                std::abs(frame[node][2]) > 10.0)
            {
                std::cout
                    << "BAD FRAME "
                    << data.frames.size()
                    << " NODE "
                    << node
                    << std::endl;
            }
        }

        data.frames.push_back(frame);

        /*
        ============================================================
        SAMPLE OUTPUT
        ============================================================
        */

        if (data.frames.size() <= 5)
        {
            std::cout
                << "\nFRAME "
                << data.frames.size() - 1
                << std::endl;

            for (size_t i = 0; i < frame.size(); i++)
            {
                std::cout
                    << "Node "
                    << i
                    << " : "
                    << frame[i][0]
                    << ", "
                    << frame[i][1]
                    << ", "
                    << frame[i][2]
                    << std::endl;
            }

            std::cout
                << "X Range: "
                << minX
                << " -> "
                << maxX
                << std::endl;

            std::cout
                << "Y Range: "
                << minY
                << " -> "
                << maxY
                << std::endl;

            std::cout
                << "Z Range: "
                << minZ
                << " -> "
                << maxZ
                << std::endl;
        }
    }

    std::cout
        << "\nTOTAL FRAMES = "
        << data.frames.size()
        << std::endl;

    if (!data.frames.empty())
    {
        std::cout
            << "NODES PER FRAME = "
            << data.frames[0].size()
            << std::endl;
    }

    return data;
}