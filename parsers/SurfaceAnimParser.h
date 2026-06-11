#pragma once

#include "base_parser.h"

#include <string>

class SurfaceAnimParser : public BaseParser
{
private:

    std::string filepath;

public:

    explicit SurfaceAnimParser(
        const std::string& path);

    PlotData parse() override;
};