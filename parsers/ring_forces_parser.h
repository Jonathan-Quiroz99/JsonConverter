#pragma once

#include "base_parser.h"

#include <string>

class RingForcesParser : public BaseParser
{
private:

    std::string filepath;

public:

    explicit RingForcesParser(
        const std::string& path);

    PlotData parse() override;
};