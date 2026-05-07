#ifndef BASE_PARSER_H
#define BASE_PARSER_H

#include <string>
#include <vector>
#include <fstream>
#include <regex>
#include <functional>
#include <map>
#include <algorithm>
#include <sstream>
#include "../config/config.h"

class BaseParser {
protected:
    std::string filepath;
    std::string filename;
    std::function<void(const std::string&)> log_callback;

    void log(const std::string& msg) {
        if (log_callback) {
            log_callback(msg);
        }
    }

public:
    BaseParser(const std::string& path, std::function<void(const std::string&)> callback = nullptr)
        : filepath(path), log_callback(callback) {
        // Extract filename from path
        size_t pos = path.find_last_of("/\\");
        filename = (pos != std::string::npos) ? path.substr(pos + 1) : path;
    }

    virtual ParsedData parse() = 0;

    // Common helper: extract numeric data
    std::vector<std::vector<double>> extract_numeric_data(bool skip_header = true,
        int max_lines = -1) {
        std::vector<std::vector<double>> numeric_data;

        std::ifstream file(filepath);
        if (!file.is_open()) {
            log("  Error opening file for numeric extraction");
            return numeric_data;
        }

        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        file.close();

        // Find data start
        size_t start_idx = 0;
        if (skip_header) {
            std::regex num_re(R"([-+]?\d*\.?\d+(?:[eEdD][-+]?\d+)?)");
            for (size_t i = 0; i < std::min(lines.size(), size_t(50)); i++) {
                if (std::regex_search(lines[i], num_re)) {
                    start_idx = i;
                    break;
                }
            }
        }

        // Process lines
        int line_count = 0;
        std::regex num_pattern(R"([-+]?\d*\.?\d+(?:[eEdD][-+]?\d+)?)");

        for (size_t i = start_idx; i < lines.size(); i++) {
            if (max_lines > 0 && line_count >= max_lines) break;

            auto words_begin = std::sregex_iterator(lines[i].begin(), lines[i].end(), num_pattern);
            auto words_end = std::sregex_iterator();

            std::vector<double> row;
            for (auto it = words_begin; it != words_end; ++it) {
                try {
                    std::string num_str = it->str();
                    for (char& c : num_str) {
                        if (c == 'D' || c == 'd') c = 'e';
                    }
                    row.push_back(std::stod(num_str));
                }
                catch (...) {
                    continue;
                }
            }

            if (!row.empty()) {
                numeric_data.push_back(row);
                line_count++;
            }
        }

        return numeric_data;
    }

    // Find section boundaries
    std::map<std::string, int> find_section_boundaries(const std::vector<std::string>& lines,
        const std::vector<std::string>& patterns) {
        std::map<std::string, int> sections;
        for (size_t i = 0; i < lines.size(); i++) {
            for (const auto& pattern : patterns) {
                std::regex re(pattern, std::regex::icase);
                if (std::regex_search(lines[i], re)) {
                    sections[lines[i]] = static_cast<int>(i);
                    break;
                }
            }
        }
        return sections;
    }
};

#endif // BASE_PARSER_H