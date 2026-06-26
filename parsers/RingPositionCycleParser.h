#pragma once

#include "base_parser.h"

#include <string>

class RingPositionCycleParser : public BaseParser
{
private:

    std::string filepath;

public:

    explicit RingPositionCycleParser(
        const std::string& path);

    PlotData parse() override;
};