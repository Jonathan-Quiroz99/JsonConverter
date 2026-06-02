#ifndef PARSER_3D_H
#define PARSER_3D_H

#include "base_parser.h"
#include "../detectors/inspection_result.h"

#include <regex>
#include <sstream>
#include <algorithm>
#include <cctype>

class Parser3D : public BaseParser
{
private:
    InspectionResult inspection;

public:

    Parser3D(
        const std::string& path,
        const InspectionResult& insp,
        std::function<void(const std::string&)> callback = nullptr)
        :
        BaseParser(path, callback),
        inspection(insp)
    {
    }

    ParsedData parse() override
    {
        log("Parsing 3D / Surface file");

        ParsedData result;

        result.filename = filename;
        result.type = inspection.type;
        result.data_type = inspection.data_type;

        /*
        ============================================================
        COPY METADATA FROM INSPECTOR
        ============================================================
        */

        result.metadata = inspection.metadata;

        /*
        ============================================================
        OPEN FILE
        ============================================================
        */

        std::ifstream file(filepath);

        if (!file.is_open())
        {
            log("Failed opening file");
            return result;
        }

        /*
        ============================================================
        SKIP HEADER
        ============================================================
        */

        std::string line;

        for (int i = 0; i < inspection.header_lines; ++i)
        {
            std::getline(file, line);
        }

        /*
        ============================================================
        PARSE NUMERIC MATRIX
        ============================================================
        */

        std::vector<std::vector<double>> matrix;

        while (std::getline(file, line))
        {
            std::vector<double> row =
                extract_numbers_from_line(line);

            if (!row.empty())
            {
                matrix.push_back(row);
            }
        }

        file.close();

        /*
        ============================================================
        VALIDATE
        ============================================================
        */

        if (matrix.empty())
        {
            log("No numeric data detected");
            return result;
        }

        /*
        ============================================================
        STORE MATRIX
        ============================================================
        */

        result.numeric_data = matrix;

        result.shape.rows =
            static_cast<int>(matrix.size());

        result.shape.cols =
            static_cast<int>(matrix[0].size());

        result.shape.is_matrix = true;

        result.shape.layout = "surface";

        /*
        ============================================================
        LOG
        ============================================================
        */

        log(
            "Rows: " +
            std::to_string(result.shape.rows));

        log(
            "Cols: " +
            std::to_string(result.shape.cols));

        log(
            "Data type: " +
            result.data_type);

        return result;
    }

private:

    std::vector<double> extract_numbers_from_line(
        const std::string& line)
    {
        std::vector<double> values;

        /*
        ============================================================
        IGNORE HEADER TEXT
        ============================================================
        */

        bool has_letters = false;

        for (char c : line)
        {
            if (
                std::isalpha(static_cast<unsigned char>(c)) &&
                c != 'E' &&
                c != 'e' &&
                c != 'D' &&
                c != 'd')
            {
                has_letters = true;
                break;
            }
        }

        if (has_letters)
        {
            return values;
        }

        /*
        ============================================================
        EXTRACT NUMBERS
        ============================================================
        */

        static const std::regex number_regex(
            R"([-+]?\d*\.?\d+(?:[eEdD][-+]?\d+)?)");

        auto begin =
            std::sregex_iterator(
                line.begin(),
                line.end(),
                number_regex);

        auto end = std::sregex_iterator();

        for (auto it = begin; it != end; ++it)
        {
            try
            {
                std::string value = it->str();

                std::replace(
                    value.begin(),
                    value.end(),
                    'D',
                    'E');

                std::replace(
                    value.begin(),
                    value.end(),
                    'd',
                    'e');

                values.push_back(
                    std::stod(value));
            }
            catch (...)
            {
            }
        }

        return values;
    }
};

#endif // PARSER_3D_H