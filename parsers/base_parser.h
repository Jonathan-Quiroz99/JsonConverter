#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <functional>
#include <algorithm>

#include "../config/config.h"

class BaseParser
{
protected:

    std::string filepath;

    std::string filename;

    std::function<void(const std::string&)>
        log_callback;

    void log(const std::string& msg)
    {
        if (log_callback)
        {
            log_callback(msg);
        }
    }

public:

    BaseParser(
        const std::string& path,
        std::function<void(const std::string&)>
        callback = nullptr)

        : filepath(path),
        log_callback(callback)
    {
        size_t pos =
            path.find_last_of("/\\");

        filename =
            (pos != std::string::npos)
            ? path.substr(pos + 1)
            : path;
    }

    virtual ParsedData parse() = 0;

protected:

    // ===== FAST NUMERIC LINE PARSER (original returning vector) =====

    static std::vector<double> parse_numeric_line(
        const std::string& line)
    {
        std::vector<double> row;

        // ===== CLEAN LINE =====

        std::string cleaned = line;

        for (char& c : cleaned)
        {
            // Replace commas with spaces
            if (c == ',')
            {
                c = ' ';
            }

            // FORTRAN scientific notation
            if (c == 'D' || c == 'd')
            {
                c = 'E';
            }
        }

        // ===== PARSE =====

        std::stringstream ss(cleaned);

        double value;

        while (ss >> value)
        {
            row.push_back(value);
        }

        return row;
    }

    // ===== OVERLOAD: fill provided vector and return success =====

    static bool parse_numeric_line(
        const std::string& line,
        std::vector<double>& outRow)
    {
        outRow = parse_numeric_line(line);
        return !outRow.empty();
    }

    // ===== STREAM PARSE =====

    std::vector<std::vector<double>>
        extract_numeric_data(
            int skip_lines = 0,
            int max_lines = -1)
    {
        std::vector<std::vector<double>>
            numeric_data;

        std::ifstream file(filepath);

        if (!file.is_open())
        {
            log("Failed opening file");

            return numeric_data;
        }

        std::string line;

        // ===== SKIP HEADER =====

        for (int i = 0; i < skip_lines; i++)
        {
            if (!std::getline(file, line))
            {
                return numeric_data;
            }
        }

        int lines_read = 0;

        // ===== STREAM READ =====

        while (std::getline(file, line))
        {
            if (max_lines > 0 &&
                lines_read >= max_lines)
            {
                break;
            }

            std::vector<double> row;

            if (parse_numeric_line(line, row))
            {
                numeric_data.push_back(
                    std::move(row));

                lines_read++;
            }
        }

        return numeric_data;
    }

    // ===== SIMPLE TEXT MATCH =====

    static bool contains(
        const std::string& text,
        const std::string& token)
    {
        return text.find(token)
            != std::string::npos;
    }

    // ===== UPPERCASE =====

    static std::string to_upper(
        std::string s)
    {
        std::transform(
            s.begin(),
            s.end(),
            s.begin(),
            ::toupper);

        return s;
    }

    // ===== TRIM =====

    static std::string trim(
        std::string s)
    {
        s.erase(
            0,
            s.find_first_not_of(
                " \t\r\n"));

        s.erase(
            s.find_last_not_of(
                " \t\r\n") + 1);

        return s;
    }
};