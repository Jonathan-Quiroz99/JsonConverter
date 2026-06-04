#pragma once

#include "base_parser.h"

#include <string>

class FaceClearanceConformParser : public BaseParser
{
private:

    std::string filepath;

public:

    explicit FaceClearanceConformParser(
        const std::string& path);

    PlotData parse() override;
};