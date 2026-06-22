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
    std::ifstream file(filepath);

    if (!file.is_open())        //not open so return
    {
        return data;
    }


	data.plotType = PlotType::Surface;
    std::string line;

    constexpr int HEADER_LINES = 5;     //init variables (header is fixed, but rows/columns are not!
    int ROWS_PER_FRAME = 101;
    int COLUMNS = 9;

    for (int i = 0; i < HEADER_LINES; i++)
    {
        std::getline(file, line);
        if (i == 2) {
            //MAYBE read the data for the axis labels
        }

        if (i == 4) {
            //get the number of nodes off the data line
            try {
                // Parse the integer from the beginning of the string
                int number = std::stoi(line);
                std::cout << "Successfully parsed: " << number << std::endl;
                ROWS_PER_FRAME = number; //
            }
            catch (const std::invalid_argument& e) {
                std::cout << "No valid conversion could be performed." << std::endl;
                return data;    //error
            }
            catch (const std::out_of_range& e) {
                std::cout << "The converted value falls out of the int range." << std::endl;
                return data;    //error
            }
        }
    }

    data.xLabel = "Circumferential Node";
    data.yLabel = "Axial Node";

    if (filepath.find("FACEPRES") != std::string::npos)
    {
        data.title = "Ring Face Pressure Cycle";
        data.zLabel = "Pressure";
        data.units = "PSI";
        //**SCOTT
        //Here, each line has the nodes as the number of columns, so probably switch???
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
    else if (filepath.find("BOT_STRESS") != std::string::npos)
    {
        data.title = "Bottom Von-Mises Stress Cycle";
        data.zLabel = "Von-Mises Stress";
        data.units = "PSI";
    }
    else if (filepath.find("BRA_CYCLE") != std::string::npos)
    {
        data.title = "Ring Bottom Side Pressure Cycle";
        data.zLabel = "Pressure";
        data.units = "PSI";
        //**SCOTT
        //Here, each line has the nodes as the number of columns, so probably switch???

    }
    else if (filepath.find("TRA_CYCLE") != std::string::npos)
    {
        data.title = "Ring Top Side Pressure Cycle";
        data.zLabel = "Pressure";
        data.units = "PSI";
        //**SCOTT
       //Here, each line has the nodes as the number of columns, so probably switch???
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