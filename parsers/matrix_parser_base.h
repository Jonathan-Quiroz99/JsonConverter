#pragma once

#include "base_parser.h"
#include "../detectors/inspection_result.h"
#include "../config/config.h"
#include <fstream>
#include <vector>
#include <string>
#include <functional>

class MatrixParserBase : public BaseParser
{
public:
    MatrixParserBase(
        const std::string& path,
        const InspectionResult& insp,
        std::function<void(const std::string&)> callback = nullptr)
        : BaseParser(path, callback),
        inspection(insp)
    {
    }

protected:
    InspectionResult inspection;

    std::vector<std::vector<double>> read_matrix(int min_cols = 1)
    {
        std::vector<std::vector<double>> matrix;

        std::ifstream file(filepath);

        if (!file.is_open())
        {
            this->log("Failed opening file");
            return matrix;
        }

        std::string line;

        for (int i = 0; i < inspection.header_lines; ++i)
        {
            if (!std::getline(file, line))
                break;
        }

        size_t expected_cols = 0;
        bool matrix_started = false;

        while (std::getline(file, line))
        {
            std::vector<double> row;

            if (!parse_numeric_line(line, row))
            {
                continue;
            }

            if (row.size() < static_cast<size_t>(min_cols))
            {
                continue;
            }

            if (!matrix_started)
            {
                expected_cols = row.size();
                matrix_started = true;
            }

            if (row.size() != expected_cols)
            {
                continue;
            }

            matrix.push_back(std::move(row));
        }

        return matrix;
    }

    bool validate_matrix(const std::vector<std::vector<double>>& m, int min_cols = 1)
    {
        if (m.empty()) return false;
        size_t cols = m[0].size();
        if (cols < static_cast<size_t>(min_cols)) return false;
        for (size_t i = 1; i < m.size(); ++i)
            if (m[i].size() != cols) return false;
        return true;
    }
};