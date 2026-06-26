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
    std::ifstream file(filepath);
    std::string line;
 
    constexpr int HEADER_LINES = 5;

    //This has two different "columns" of data to be graphed together
    Series raw;
    raw.name =        "RAW";
    Series filtered;
    filtered.name =   "Filtered";

    data.plotType =   PlotType::Scatter;
    data.title =      "Ring Strain Energy During Conformance";
    data.xLabel =     "Output Step";
    data.yLabel =     "Strain Energy";
    data.units =      "in-lb";


    if (!file.is_open())  return data;        //Not open, so return

    for (int i = 0; i < HEADER_LINES; i++)
    {
        std::getline(file, line);
        if (i == 0) {
            std::string old_word = "THIS FILE CONTAINS THE";
            std::string new_word = "";

            // Find the starting position of the target word
            size_t pos = line.find(old_word);

            // Check if the word was actually found
            if (pos != std::string::npos) {
                // Parameters: (starting position, character count to erase, replacement string)
                line.replace(pos, old_word.length(), new_word);
            }

            //Set title
            data.title = line + " CONFORMANCE"; //missing word in title inside file!!!
            data.subtitle = "";

        }
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