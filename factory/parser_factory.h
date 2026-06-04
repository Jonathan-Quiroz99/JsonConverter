#pragma once

#include <memory>
#include <string>

#include "../parsers/base_parser.h"

class ParserFactory
{
public:

    static std::unique_ptr<BaseParser>
        create(
            const std::string& filepath);
};