#pragma once

#include "base_parser.h"

#include <string>

class StrainEnergyConformParser : public BaseParser
{
private:

    std::string filepath;

public:

    explicit StrainEnergyConformParser(
        const std::string& path);

    PlotData parse() override;
};