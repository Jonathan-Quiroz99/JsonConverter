#ifndef PARSER_GENERIC_H
#define PARSER_GENERIC_H

#include "base_parser.h"
#include <cctype>
#include <algorithm>
#include <set>

class ParserGeneric : public BaseParser {
public:
    ParserGeneric(const std::string& path, std::function<void(const std::string&)> callback = nullptr)
        : BaseParser(path, callback) {
    }

    ParsedData parse() override {
        log("  Generic parsing mode - analyzing file structure");

        ParsedData result;
        result.filename = filename;

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

        // Step 1: Analyze file structure
        FileAnalysis analysis = analyze_file_structure(lines);

        // Step 2: Extract metadata from headers
        result.metadata = extract_metadata(lines, analysis);

        // Step 3: Parse based on structure
        if (analysis.has_matrix_data) {
            result = parse_matrix_data(lines, analysis, result);
            result.type = FileType::GRID_2D;
            result.shape.is_matrix = true;
            result.shape.layout = "grid";
        }
        else if (analysis.has_columnar_data) {
            result = parse_columnar_data(lines, analysis, result);
            result.type = FileType::TIME_SERIES_2D;
            result.shape.layout = "columnar";
        }
        else if (analysis.has_sections) {
            result = parse_sectioned_data(lines, analysis, result);
            result.shape.layout = "sections";
        }
        else {
            // Last resort: try to extract any numeric data
            result.numeric_data = extract_numeric_data(true);
            if (!result.numeric_data.empty()) {
                result.type = FileType::TIME_SERIES_2D;
                analyze_data_shape(result);
            }
        }

        // Log summary
        log_summary(result);

        return result;
    }

private:
    struct FileAnalysis {
        bool has_matrix_data = false;
        bool has_columnar_data = false;
        bool has_sections = false;
        bool has_headers = false;
        int header_lines = 0;
        int data_start_line = -1;
        size_t numeric_columns = 0;
        size_t numeric_rows = 0;
        std::vector<size_t> column_counts; // For detecting ragged arrays
        std::map<std::string, std::vector<int>> section_boundaries;
        std::set<std::string> keywords_found;

        // Data characteristics
        bool has_scientific_notation = false;
        bool has_negative_numbers = false;
        double min_value = 0, max_value = 0;
    };

    FileAnalysis analyze_file_structure(const std::vector<std::string>& lines) {
        FileAnalysis analysis;

        std::regex num_re(R"([-+]?\d*\.?\d+(?:[eEdD][-+]?\d+)?)");
        std::regex scientific_re(R"([eEdD][-+]?\d+)", std::regex::icase);

        std::vector<std::vector<double>> all_rows;
        std::vector<int> line_types; // 0=text, 1=numeric

        for (size_t i = 0; i < lines.size() && i < 500; i++) { // Analyze first 500 lines
            const auto& line = lines[i];
            if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos) continue;

            auto words_begin = std::sregex_iterator(line.begin(), line.end(), num_re);
            auto words_end = std::sregex_iterator();

            std::vector<double> numbers;
            bool has_text = false;
            bool has_sci = false;

            for (auto it = words_begin; it != words_end; ++it) {
                try {
                    std::string num_str = it->str();
                    if (std::regex_search(num_str, scientific_re)) has_sci = true;
                    for (char& c : num_str) if (c == 'D' || c == 'd') c = 'e';
                    numbers.push_back(std::stod(num_str));
                    if (numbers.back() < 0) analysis.has_negative_numbers = true;
                }
                catch (...) { continue; }
            }

            // Check if line contains letters (text)
            for (char c : line) {
                if (std::isalpha(static_cast<unsigned char>(c)) && c != 'e' && c != 'E' && c != 'd' && c != 'D') {
                    has_text = true;
                    break;
                }
            }

            if (!numbers.empty()) {
                analysis.numeric_rows++;
                all_rows.push_back(numbers);
                analysis.column_counts.push_back(numbers.size());
                line_types.push_back(1);

                if (has_sci) analysis.has_scientific_notation = true;

                // Track min/max
                for (double val : numbers) {
                    if (val < analysis.min_value) analysis.min_value = val;
                    if (val > analysis.max_value) analysis.max_value = val;
                }
            }
            else if (has_text) {
                line_types.push_back(0);
                // Extract keywords for metadata
                std::string upper_line = line;
                std::transform(upper_line.begin(), upper_line.end(), upper_line.begin(), ::toupper);
                std::istringstream iss(upper_line);
                std::string word;
                while (iss >> word) {
                    if (word.length() > 3) {
                        analysis.keywords_found.insert(word);
                    }
                }
            }
        }

        // Determine data structure
        if (all_rows.size() > 1) {
            // Check if it's a matrix (consistent columns)
            bool consistent_cols = true;
            size_t first_cols = all_rows[0].size();
            for (size_t i = 1; i < std::min(all_rows.size(), size_t(20)); i++) {
                if (all_rows[i].size() != first_cols) {
                    consistent_cols = false;
                    break;
                }
            }

            analysis.has_matrix_data = consistent_cols && all_rows.size() > 3 && first_cols > 1;
            analysis.has_columnar_data = !analysis.has_matrix_data && first_cols <= 5;

            if (analysis.has_matrix_data) {
                analysis.numeric_columns = first_cols;
            }
        }

        // Find data start line (first numeric line after headers)
        for (size_t i = 0; i < line_types.size(); i++) {
            if (line_types[i] == 1) {
                // Check if previous lines had headers
                bool has_headers_before = false;
                for (size_t j = 0; j < i && j < 20; j++) {
                    if (line_types[j] == 0) {
                        has_headers_before = true;
                        break;
                    }
                }
                if (has_headers_before || i > 5) {
                    analysis.data_start_line = static_cast<int>(i);
                    analysis.header_lines = static_cast<int>(i);
                    break;
                }
            }
        }

        // Detect sections (e.g., "FILM THICKNESS", "PRESSURE", etc.)
        std::vector<std::string> section_patterns = {
            "FILM", "THICKNESS", "PRESSURE", "TEMPERATURE", "STRESS", "WEAR",
            "SECTION", "COMPONENT", "RING", "LOAD", "DISPLACEMENT"
        };

        for (size_t i = 0; i < lines.size() && i < 200; i++) {
            std::string upper = lines[i];
            std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
            for (const auto& pattern : section_patterns) {
                if (upper.find(pattern) != std::string::npos && upper.find_first_of("0123456789") == std::string::npos) {
                    analysis.has_sections = true;
                    analysis.section_boundaries[upper].push_back(static_cast<int>(i));
                    break;
                }
            }
        }

        return analysis;
    }

    std::map<std::string, std::string> extract_metadata(const std::vector<std::string>& lines, const FileAnalysis& analysis) {
        std::map<std::string, std::string> metadata;

        // Extract from first few lines (headers)
        int max_header_lines = (analysis.header_lines > 0) ? analysis.header_lines : 20;

        for (int i = 0; i < std::min(static_cast<int>(lines.size()), max_header_lines); i++) {
            const auto& line = lines[i];

            // Look for key:value patterns
            std::regex kv_re(R"(([A-Z][A-Z\s]+):\s*(.+))", std::regex::icase);
            std::smatch match;
            if (std::regex_search(line, match, kv_re)) {
                std::string key = match[1];
                std::string value = match[2];
                // Trim
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                metadata[key] = value;
            }

            // Look for units
            std::regex units_re(R"(UNITS?:?\s*([A-Z\xC5\xB5]+))", std::regex::icase);
            if (std::regex_search(line, match, units_re)) {
                metadata["units"] = match[1];
            }

            // Look for time/duration
            std::regex time_re(R"((TIME|DURATION|HOURS?):?\s*(\d+\.?\d*))", std::regex::icase);
            if (std::regex_search(line, match, time_re)) {
                metadata["time"] = match[2];
            }
        }

        // Add analysis metadata
        if (analysis.has_scientific_notation) metadata["format"] = "scientific_notation";
        if (analysis.has_negative_numbers) metadata["has_negatives"] = "true";
        metadata["min_value"] = std::to_string(analysis.min_value);
        metadata["max_value"] = std::to_string(analysis.max_value);
        metadata["total_numeric_rows"] = std::to_string(analysis.numeric_rows);

        return metadata;
    }

    ParsedData parse_matrix_data(const std::vector<std::string>& lines, const FileAnalysis& analysis, ParsedData& result) {
        log("  Detected matrix/grid data structure");

        // Extract numeric data starting from data_start_line
        std::vector<std::vector<double>> matrix;
        std::regex num_re(R"([-+]?\d*\.?\d+(?:[eEdD][-+]?\d+)?)");

        int start_line = (analysis.data_start_line >= 0) ? analysis.data_start_line : 0;

        for (size_t i = start_line; i < lines.size(); i++) {
            auto words_begin = std::sregex_iterator(lines[i].begin(), lines[i].end(), num_re);
            auto words_end = std::sregex_iterator();

            std::vector<double> row;
            for (auto it = words_begin; it != words_end; ++it) {
                try {
                    std::string num_str = it->str();
                    for (char& c : num_str) if (c == 'D' || c == 'd') c = 'e';
                    row.push_back(std::stod(num_str));
                }
                catch (...) { continue; }
            }

            if (!row.empty()) {
                matrix.push_back(row);
            }
        }

        result.numeric_data = matrix;
        result.shape.rows = matrix.size();
        result.shape.cols = matrix.empty() ? 0 : matrix[0].size();

        // Determine data type from keywords
        if (analysis.keywords_found.find("WEAR") != analysis.keywords_found.end()) {
            result.data_type = "wear_distribution";
        }
        else if (analysis.keywords_found.find("PRESSURE") != analysis.keywords_found.end()) {
            result.data_type = "pressure_distribution";
        }
        else if (analysis.keywords_found.find("TEMPERATURE") != analysis.keywords_found.end()) {
            result.data_type = "temperature_distribution";
        }
        else if (analysis.keywords_found.find("STRESS") != analysis.keywords_found.end()) {
            result.data_type = "stress_distribution";
        }
        else if (analysis.keywords_found.find("FILM") != analysis.keywords_found.end() ||
            analysis.keywords_found.find("THICKNESS") != analysis.keywords_found.end()) {
            result.data_type = "film_thickness";
        }
        else {
            result.data_type = "matrix_data";
        }

        log("  Matrix dimensions: " + std::to_string(result.shape.rows) + " x " + std::to_string(result.shape.cols));

        return result;
    }

    ParsedData parse_columnar_data(const std::vector<std::string>& lines, const FileAnalysis& analysis, ParsedData& result) {
        log("  Detected columnar/time-series data structure");

        result.numeric_data = extract_numeric_data(true);
        result.shape.rows = result.numeric_data.size();
        result.shape.cols = result.numeric_data.empty() ? 0 : result.numeric_data[0].size();

        // Determine if it's time series or correlation
        if (result.shape.cols == 2 && result.numeric_data.size() > 10) {
            bool increasing = true;
            for (size_t i = 1; i < std::min(result.numeric_data.size(), size_t(10)); i++) {
                if (result.numeric_data[i][0] <= result.numeric_data[i - 1][0]) {
                    increasing = false;
                    break;
                }
            }
            result.type = increasing ? FileType::TIME_SERIES_2D : FileType::CORRELATION_2D;
        }

        // Determine data type from keywords
        if (analysis.keywords_found.find("POSITION") != analysis.keywords_found.end()) {
            result.data_type = "position";
        }
        else if (analysis.keywords_found.find("VELOCITY") != analysis.keywords_found.end()) {
            result.data_type = "velocity";
        }
        else if (analysis.keywords_found.find("ACCELERATION") != analysis.keywords_found.end()) {
            result.data_type = "acceleration";
        }
        else if (analysis.keywords_found.find("ANGLE") != analysis.keywords_found.end()) {
            result.data_type = "angle";
        }
        else {
            result.data_type = "time_series";
        }

        log("  Columnar data: " + std::to_string(result.shape.rows) + " rows, " +
            std::to_string(result.shape.cols) + " columns");

        return result;
    }

    ParsedData parse_sectioned_data(const std::vector<std::string>& lines, const FileAnalysis& analysis, ParsedData& result) {
        log("  Detected sectioned data structure");

        // Similar to original Parser3D but more flexible
        std::map<std::string, ParsedData::SectionData> sections;
        std::string current_section;
        int current_component = 0;
        std::vector<std::vector<double>> component_data;

        std::regex num_re(R"([-+]?\d*\.?\d+(?:[eEdD][-+]?\d+)?)");

        for (size_t i = 0; i < lines.size(); i++) {
            std::string line = lines[i];
            std::string upper = line;
            std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

            // Check if this line starts a new section
            bool is_new_section = false;
            for (const auto& [section_name, positions] : analysis.section_boundaries) {
                for (int pos : positions) {
                    if (pos == static_cast<int>(i)) {
                        // Save previous component
                        if (!component_data.empty() && !current_section.empty()) {
                            sections[current_section].components[current_component] = component_data;
                            component_data.clear();
                        }
                        current_section = section_name;
                        current_component = 1;
                        sections[current_section] = ParsedData::SectionData();
                        is_new_section = true;
                        log("  Section: " + current_section);
                        break;
                    }
                }
                if (is_new_section) break;
            }
            if (is_new_section) continue;

            // Check for component number
            std::regex comp_re(R"(COMPONENT\s*NUMBER\s*:?\s*(\d+))", std::regex::icase);
            std::regex ring_re(R"(RING\s*NUMBER\s*:?\s*(\d+))", std::regex::icase);
            std::smatch match;

            if (std::regex_search(line, match, comp_re) || std::regex_search(line, match, ring_re)) {
                if (!component_data.empty() && !current_section.empty()) {
                    sections[current_section].components[current_component] = component_data;
                    component_data.clear();
                }
                try {
                    current_component = std::stoi(match[1]);
                }
                catch (...) {
                    current_component++;
                }
                continue;
            }

            // Extract numeric data
            auto words_begin = std::sregex_iterator(line.begin(), line.end(), num_re);
            auto words_end = std::sregex_iterator();

            std::vector<double> row;
            for (auto it = words_begin; it != words_end; ++it) {
                try {
                    std::string num_str = it->str();
                    for (char& c : num_str) if (c == 'D' || c == 'd') c = 'e';
                    row.push_back(std::stod(num_str));
                }
                catch (...) { continue; }
            }

            if (!row.empty() && !current_section.empty()) {
                component_data.push_back(row);
            }
        }

        // Save last component
        if (!component_data.empty() && !current_section.empty()) {
            sections[current_section].components[current_component] = component_data;
        }

        result.sections = sections;
        result.data_type = "sectioned_data";

        log("  Found " + std::to_string(sections.size()) + " sections");

        return result;
    }

    void analyze_data_shape(ParsedData& data) {
        if (data.numeric_data.empty()) return;

        data.shape.rows = data.numeric_data.size();
        data.shape.cols = data.numeric_data[0].size();

        // Check if matrix (consistent columns)
        data.shape.is_matrix = true;
        for (size_t i = 1; i < std::min(data.numeric_data.size(), size_t(20)); i++) {
            if (data.numeric_data[i].size() != data.shape.cols) {
                data.shape.is_matrix = false;
                break;
            }
        }
    }

    void log_summary(const ParsedData& data) {
        if (data.has_2d_data()) {
            log("  Summary: " + std::to_string(data.numeric_data.size()) + " rows, " +
                (data.numeric_data.empty() ? "0" : std::to_string(data.numeric_data[0].size())) + " cols");
        }
        if (data.has_sections()) {
            size_t total_comps = 0;
            for (const auto& sec : data.sections) {
                total_comps += sec.second.components.size();
            }
            log("  Summary: " + std::to_string(data.sections.size()) + " sections, " +
                std::to_string(total_comps) + " components");
        }
        if (data.has_mesh()) {
            log("  Summary: " + std::to_string(data.mesh.x.size()) + " vertices, " +
                std::to_string(data.mesh.i.size()) + " triangles");
        }
    }
};

#endif // PARSER_GENERIC_H