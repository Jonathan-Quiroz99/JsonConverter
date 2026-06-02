#pragma once

#include "matrix_parser_base.h"
#include "../detectors/inspection_result.h"
#include "../config/config.h"
#include <functional>
#include <string>

class ParserHeatmap : public MatrixParserBase
{
public:
    ParserHeatmap(
        const std::string& path,
        const InspectionResult& insp,
        std::function<void(const std::string&)> callback = nullptr)
        : MatrixParserBase(path, insp, callback)
    {
    }

    ParsedData parse() override
    {
        this->log("ParserHeatmap: parsing matrix-based heatmap");

        ParsedData out;

        out.filename = this->filename;

        out.metadata = this->inspection.metadata;

        auto matrix = this->read_matrix(1);

        if (!this->validate_matrix(matrix, 1) || matrix.empty())
        {
            this->log("ParserHeatmap: invalid or empty matrix");
            return out;
        }

        out.data_type = this->inspection.data_type;

        if (out.data_type == "pressure")
        {
            out.metadata["x_axis"] =
                "Circumferential Node";

            out.metadata["y_axis"] =
                "Radial Node";
        }

        else if (out.data_type == "stress")
        {
            out.metadata["x_axis"] =
                "Surface Node";

            out.metadata["y_axis"] =
                "Element";
        }

        out.numeric_data = matrix;

        out.shape.rows =
            static_cast<size_t>(matrix.size());

        out.shape.cols =
            matrix[0].size();

        out.shape.is_matrix = true;

        out.shape.layout = "matrix";

        out.plot_type = "heatmap";

        this->log(
            "ParserHeatmap: rows=" +
            std::to_string(out.shape.rows) +
            " cols=" +
            std::to_string(out.shape.cols));

        return out;
    }
};