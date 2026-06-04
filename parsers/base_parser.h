#pragma once

#include "../models/plot_data.h"

class BaseParser
{
public:

    virtual PlotData parse() = 0;

    virtual ~BaseParser() = default;
};