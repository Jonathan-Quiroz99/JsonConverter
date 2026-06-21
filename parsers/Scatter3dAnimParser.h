#pragma once

#include "base_parser.h"

#include <string>

class Scatter3dAnimParser : public BaseParser
{
private:

    std::string filepath;

public:

    explicit Scatter3dAnimParser(
        const std::string& path);

    PlotData parse() override;
};