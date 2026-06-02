#ifndef PARSER_2D_H
#define PARSER_2D_H

#include "base_parser.h"
#include "../detectors/inspection_result.h"
#include <fstream>

class Parser2D : public BaseParser
{
public:

    Parser2D(
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
        log("Parsing 2D file");

        ParsedData result;

        result.filename = filename;
        result.type = inspection_info.type;
        result.data_type = inspection_info.data_type;
        result.plot_type = inspection_info.metadata.count("plot_type") ? inspection_info.metadata.at("plot_type") : std::string("scatter");
        result.metadata = inspection_info.metadata;

        std::ifstream file(filepath);

        if (!file.is_open())
        {
            log("Failed opening file");
            return result;
        }

        // skip header lines if any
        std::string line;
        for (int i = 0; i < inspection_info.header_lines; ++i)
        {
            if (!std::getline(file, line)) break;
        }

        // read numeric pairs / rows
        while (std::getline(file, line))
        {
            std::vector<double> row;
            if (parse_numeric_line(line, row))
            {
                result.numeric_data.push_back(row);
            }
        }

        file.close();

        result.shape.rows = result.numeric_data.size();
        result.shape.cols = result.numeric_data.empty() ? 0 : result.numeric_data[0].size();
        result.shape.is_matrix = (result.shape.rows > 0 && result.shape.cols > 0 && result.shape.rows > 1 && result.shape.cols > 1 && result.shape.cols != 2);
        result.shape.layout = (result.shape.cols == 2) ? "2d" : "matrix";

        log("Rows: " + std::to_string(result.shape.rows));
        log("Cols: " + std::to_string(result.shape.cols));
        log("Data type: " + result.data_type);

        return result;
    }

private:

    InspectionResult inspection_info;
};

#endif // PARSER_2D_H