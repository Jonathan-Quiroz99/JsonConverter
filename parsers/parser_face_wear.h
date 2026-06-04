#pragma once

#include "base_parser.h"

#include <string>

class FaceWearParser : public BaseParser
{
private:

    std::string filepath;

public:

    explicit FaceWearParser(
        const std::string& path);

    PlotData parse() override;
};