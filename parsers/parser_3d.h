#ifndef PARSER_3D_H
#define PARSER_3D_H

#include "base_parser.h"
#include <cctype>
#include <algorithm>
#include <sstream>

class Parser3D : public BaseParser {
public:
    Parser3D(const std::string& path, std::function<void(const std::string&)> callback = nullptr)
        : BaseParser(path, callback) {
    }

    ParsedData parse() override {
        log("  Parsing as 3D data");

        ParsedData result;
        result.filename = filename;
        result.type = FileType::MESH_3D_SURFACE;

        std::ifstream file(filepath);
        if (!file.is_open()) {
            log("  Error opening file");
            return result;
        }

        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        file.close();

        // Detect data type
        std::string data_type = detect_data_type(lines);
        result.data_type = data_type;
        log("  Data type: " + data_type);

        // Parse sections
        std::map<std::string, ParsedData::SectionData> sections = parse_sections(lines);

        // Count data
        size_t total_components = 0;
        size_t total_rows = 0;
        for (const auto& sec : sections) {
            total_components += sec.second.components.size();
            for (const auto& comp : sec.second.components) {
                total_rows += comp.second.size();
            }
        }

        log("  Sections: " + std::to_string(sections.size()));
        log("  Total components: " + std::to_string(total_components));
        log("  Data rows: " + std::to_string(total_rows));

        if (total_rows == 0 && sections.empty()) {
            // Try as simple 2D grid
            log("  No sections detected, trying as grid");
            result.numeric_data = extract_numeric_data(true);
            if (!result.numeric_data.empty()) {
                log("  Parsed as grid with " + std::to_string(result.numeric_data.size()) + " rows");
                result.type = FileType::GRID_2D;
            }
        }
        else {
            result.sections = sections;
        }

        return result;
    }

private:
    std::string detect_data_type(const std::vector<std::string>& lines) {
        std::string content;
        for (size_t i = 0; i < min(lines.size(), size_t(50)); i++) {
            std::string upper = lines[i];
            transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
            content += upper + " ";
        }

        if (content.find("OIL FILM THICKNESS") != std::string::npos) {
            return "oil_film_thickness";
        }
        else if (content.find("CONTACT PRESSURE") != std::string::npos) {
            return "pressure";
        }
        else if (content.find("TEMPERATURE") != std::string::npos) {
            return "temperature";
        }
        else if (content.find("VON-MISES STRESS") != std::string::npos ||
            content.find("STRESS") != std::string::npos) {
            return "stress";
        }
        else if (content.find("WEAR") != std::string::npos) {
            return "wear";
        }

        // Check filename
        std::string fname_lower = filename;
        transform(fname_lower.begin(), fname_lower.end(), fname_lower.begin(), ::tolower);

        if (fname_lower.find("film") != std::string::npos) {
            return "oil_film_thickness";
        }
        else if (fname_lower.find("pres") != std::string::npos) {
            return "pressure";
        }
        else if (fname_lower.find("stress") != std::string::npos) {
            return "stress";
        }
        else if (fname_lower.find("wear") != std::string::npos) {
            return "wear";
        }

        return "generic_3d";
    }

    std::map<std::string, ParsedData::SectionData> parse_sections(const std::vector<std::string>& lines) {
        std::map<std::string, ParsedData::SectionData> sections;
        std::string current_section;
        int current_component = -1;
        std::vector<std::vector<double>> component_data;

        // Section detection patterns
        std::vector<std::string> patterns = {
            R"(FILM.*THICKNESS)",
            R"(PRESSURE)",
            R"(TEMPERATURE)",
            R"(STRESS)",
            R"(WEAR)"
        };

        std::regex num_re(R"([-+]?\d*\.?\d+(?:[eEdD][-+]?\d+)?)");

        for (size_t i = 0; i < lines.size(); i++) {
            std::string line = lines[i];
            // Trim whitespace
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            if (line.empty()) continue;

            // Check for section header
            bool is_section = false;
            for (const auto& pattern : patterns) {
                std::regex re(pattern, std::regex::icase);
                if (std::regex_match(line, re)) {
                    // Save previous component
                    if (!component_data.empty()) {
                        sections[current_section].components[current_component] = component_data;
                        component_data.clear();
                    }
                    current_section = line;
                    current_component = 1;
                    sections[current_section] = ParsedData::SectionData();
                    log("  New section: " + current_section);
                    is_section = true;
                    break;
                }
            }
            if (is_section) continue;

            // Check for component header
            if (std::regex_search(line, std::regex(R"(RING\s*NUMBER|COMPONENT\s*NUMBER)", std::regex::icase))) {
                // Save previous component
                if (!component_data.empty() && current_component > 0) {
                    sections[current_section].components[current_component] = component_data;
                    component_data.clear();
                }

                // Extract component number
                size_t colon_pos = line.find(':');
                if (colon_pos != std::string::npos) {
                    std::string num_str = line.substr(colon_pos + 1);
                    try {
                        current_component = stoi(num_str);
                    }
                    catch (...) {
                        current_component++;
                    }
                }
                else {
                    current_component++;
                }
                log("  Component: " + std::to_string(current_component));
                continue;
            }

            // Try to extract numeric data
            auto words_begin = sregex_iterator(line.begin(), line.end(), num_re);
            auto words_end = sregex_iterator();

            std::vector<double> row;
            for (auto it = words_begin; it != words_end; ++it) {
                try {
                    std::string num_str = it->str();
                    for (char& c : num_str) {
                        if (c == 'D' || c == 'd') c = 'e';
                    }
                    row.push_back(stod(num_str));
                }
                catch (...) {}
            }

            if (!row.empty()) {
                // Check if line contains letters (descriptive text)
                bool has_letters = false;
                for (char c : line) {
                    if (isalpha(c) && c != 'e' && c != 'E' && c != 'd' && c != 'D') {
                        has_letters = true;
                        break;
                    }
                }
                if (has_letters) continue;

                if (current_component < 0) {
                    current_component = 1;
                }
                component_data.push_back(row);
            }
        }

        // Save last component
        if (!component_data.empty() && !current_section.empty()) {
            sections[current_section].components[current_component] = component_data;
        }

        return sections;
    }
};

#endif // PARSER_3D_H