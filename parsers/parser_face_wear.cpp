#include "parser_face_wear.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

FaceWearParser::FaceWearParser(
    const std::string& path)
    :
    filepath(path)
{
}

PlotData FaceWearParser::parse()
{
    PlotData data;
    std::string line;
    constexpr int HEADER_LINES = 5;
    std::ifstream file(filepath);

    if (!file.is_open())  return data;        //Not open, so return

    data.plotType = PlotType::Surface;

    data.title =        "Ring Wear";
    data.xLabel =        "Circumferential Node";
    data.yLabel =        "Axial Node";
    data.zLabel =        "Wear (um)";
    data.units = "um";

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
            data.title = line;
            data.subtitle = "";

        }
        if (i == 2) {
            //Probably better to read the data for the axis labels
        }
        if (i == 4) {
            //In the WEAR files, the time is on the 5th (i=4) line, so append it to title, but remove duplicate word.
            std::string old_word = "WEAR";
            std::string new_word = "";

            // Find the starting position of the target word
            size_t pos = line.find(old_word);

            // Check if the word was actually found
            if (pos != std::string::npos) {
                // Parameters: (starting position, character count to erase, replacement string)
                line.replace(pos, old_word.length(), new_word);
            }
            //Set title
            data.title = data.title + line;
        }
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