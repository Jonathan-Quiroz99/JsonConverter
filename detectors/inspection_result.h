#pragma once

#include <string>
#include <vector>
#include <map>

#include "../config/config.h"

struct InspectionResult
{
    FileType type = FileType::UNKNOWN;

    std::string data_type;

    int header_lines = 0;

    bool is_matrix = false;

    size_t rows = 0;
    size_t cols = 0;

    std::vector<std::string> metadata_lines;

    std::map<std::string, std::string> metadata;
};