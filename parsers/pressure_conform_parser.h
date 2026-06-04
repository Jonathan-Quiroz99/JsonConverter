#pragma once

#include "base_parser.h"

#include <string>

class PressureConformParser : public BaseParser
{
private:

    std::string filepath;

public:

    explicit PressureConformParser(
        const std::string& path);

    PlotData parse() override;
};