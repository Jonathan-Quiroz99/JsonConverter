#pragma once

#include "base_parser.h"

#include <string>

class PositionConformParser : public BaseParser
{
private:

    std::string filepath;

public:

    explicit PositionConformParser(
        const std::string& path);

    PlotData parse() override;
};