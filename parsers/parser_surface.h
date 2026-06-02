#ifndef PARSER_SURFACE_H
#define PARSER_SURFACE_H

#include <fstream>
#include <string>
#include <vector>
#include <functional>

#include "base_parser.h"
#include "../detectors/inspection_result.h"

class ParserSurface : public BaseParser
{
public:

    ParserSurface(
        const std::string& path,
        const InspectionResult& inspection,
        std::function<void(const std::string&)> callback = nullptr)
        :
        BaseParser(path, callback),
        inspection_info(inspection)
    {
    }

    ParsedData parse() override
    {
        log("Parsing surface file");

        ParsedData result;

        result.filename = filename;
        result.type = inspection_info.type;
        result.data_type = inspection_info.data_type;
        result.plot_type = inspection_info.metadata.count("plot_type") ? inspection_info.metadata.at("plot_type") : std::string("surface");
        result.metadata = inspection_info.metadata;

        std::ifstream file(filepath);

        if (!file.is_open())
        {
            log("Failed opening file");

            return result;
        }

        std::string line;

        // Skip header lines
        for (int i = 0; i < inspection_info.header_lines; ++i)
        {
            if (!std::getline(file, line)) break;
        }

        // Read matrix rows
        size_t expected_cols = 0;

        while (std::getline(file, line))
        {
            std::vector<double> row;

            if (parse_numeric_line(line, row))
            {
                if (row.size() < 20)
                {
                    continue;
                }

                if (expected_cols == 0)
                {
                    expected_cols = row.size();
                }

                if (row.size() != expected_cols)
                {
                    continue;
                }

                result.numeric_data.push_back(
                    std::move(row));
            }
        }

        file.close();

        result.shape.rows =
            result.numeric_data.size();

        if (!result.numeric_data.empty())
        {
            result.shape.cols =
                result.numeric_data[0].size();
        }

        result.shape.is_matrix = true;

        result.shape.layout = "grid";

        log(
            "Rows: " +
            std::to_string(result.shape.rows));

        log(
            "Cols: " +
            std::to_string(result.shape.cols));

        return result;
    }

private:

    InspectionResult inspection_info;
};

#endif // PARSER_SURFACE_H