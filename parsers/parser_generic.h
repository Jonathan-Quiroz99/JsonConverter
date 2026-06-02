#ifndef PARSER_GENERIC_H
#define PARSER_GENERIC_H

#include "base_parser.h"
#include "../detectors/inspection_result.h"

#include <regex>
#include <set>
#include <algorithm>
#include <sstream>

class ParserGeneric : public BaseParser
{
private:

    InspectionResult inspection;

public:

    ParserGeneric(
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
        log("Generic parser started");

        ParsedData result;

        result.filename = filename;
        result.type = inspection.type;
        result.data_type = inspection.data_type;
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
        SKIP HEADERS
        ============================================================
        */

        std::string line;

        for (int i = 0; i < inspection.header_lines; ++i)
        {
            std::getline(file, line);
        }

        /*
        ============================================================
        PARSE NUMERIC DATA
        ============================================================
        */

        std::vector<std::vector<double>> rows;

        while (std::getline(file, line))
        {
            std::vector<double> values =
                extract_numbers_from_line(line);

            if (!values.empty())
            {
                rows.push_back(values);
            }
        }

        file.close();

        /*
        ============================================================
        VALIDATE
        ============================================================
        */

        if (rows.empty())
        {
            log("No numeric data detected");
            return result;
        }

        /*
        ============================================================
        STORE
        ============================================================
        */

        result.numeric_data = rows;

        result.shape.rows =
            static_cast<int>(rows.size());

        result.shape.cols =
            static_cast<int>(rows[0].size());

        result.shape.is_matrix =
            is_matrix(rows);

        /*
        ============================================================
        AUTO LAYOUT
        ============================================================
        */

        if (result.shape.is_matrix)
        {
            result.shape.layout = "matrix";
        }
        else if (result.shape.cols == 2)
        {
            result.shape.layout = "2d";
        }
        else if (result.shape.cols == 3)
        {
            result.shape.layout = "3d";
        }
        else
        {
            result.shape.layout = "generic";
        }

        /*
        ============================================================
        DETECT TYPE IF UNKNOWN
        ============================================================
        */

        if (result.data_type.empty())
        {
            result.data_type =
                detect_data_type(result);
        }

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
            "Matrix: " +
            std::string(
                result.shape.is_matrix
                ? "true"
                : "false"));

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
        IGNORE TEXT LINES
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

    bool is_matrix(
        const std::vector<std::vector<double>>& rows)
    {
        if (rows.empty())
        {
            return false;
        }

        size_t cols = rows[0].size();

        for (size_t i = 1; i < rows.size(); ++i)
        {
            if (rows[i].size() != cols)
            {
                return false;
            }
        }

        return true;
    }

    std::string detect_data_type(
        const ParsedData& data)
    {
        /*
        ============================================================
        USE METADATA FIRST
        ============================================================
        */

        std::string combined;

        for (const auto& item : inspection.metadata)
        {
            combined += item.first + " ";
            combined += item.second + " ";
        }

        std::transform(
            combined.begin(),
            combined.end(),
            combined.begin(),
            ::toupper);

        /*
        ============================================================
        DETECT TYPE
        ============================================================
        */

        if (combined.find("STRESS") != std::string::npos)
        {
            return "stress";
        }

        if (combined.find("PRESSURE") != std::string::npos)
        {
            return "pressure";
        }

        if (combined.find("TEMPERATURE") != std::string::npos)
        {
            return "temperature";
        }

        if (combined.find("WEAR") != std::string::npos)
        {
            return "wear";
        }

        if (combined.find("FILM") != std::string::npos)
        {
            return "film_thickness";
        }

        /*
        ============================================================
        FALLBACK USING SHAPE
        ============================================================
        */

        if (data.shape.cols == 2)
        {
            return "2d_series";
        }

        if (data.shape.cols == 3)
        {
            return "3d_points";
        }

        if (data.shape.is_matrix)
        {
            return "matrix_data";
        }

        return "generic_data";
    }
};

#endif // PARSER_GENERIC_H