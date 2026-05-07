#ifndef FILE_DETECTOR_H
#define FILE_DETECTOR_H

#include <string>
#include <vector>
#include <regex>
#include <fstream>
#include <functional>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <cctype>
#include "../config/config.h"

namespace fs = std::filesystem;

class FileDetector {
public:
    static FileType detect(const std::string& filepath,
        std::function<void(const std::string&)> log_callback = nullptr) {

        auto log = [&](const std::string& msg) {
            if (log_callback) log_callback(msg);
            };

        // File existence check
        try {
            if (!fs::exists(filepath) || !fs::is_regular_file(filepath)) {
                log("  Error: File not found or not a regular file (" + filepath + ")");
                return FileType::UNKNOWN;
            }
        }
        catch (...) {
            log("  Warning: filesystem checks failed for path: " + filepath);
        }

        std::ifstream file(filepath);
        if (!file.is_open()) {
            log("  Error opening file: " + filepath);
            return FileType::UNKNOWN;
        }

        // Read file content (first 200 lines for better detection)
        std::string content;
        std::string line;
        int line_count = 0;
        std::vector<std::string> lines;

        while (std::getline(file, line) && line_count < 200) {
            content += line + "\n";
            lines.push_back(line);
            line_count++;
        }
        file.close();

        // ===== STEP 1: Pattern-based detection (specific formats) =====
        struct Pattern {
            FileType type;
            std::string regex_pattern;
            std::string description;
        };

        std::vector<Pattern> patterns = {
            // Gmsh mesh files (highest priority)
            {FileType::MESH_3D_VOLUME, R"(\$MeshFormat)", "Gmsh mesh format"},
            {FileType::MESH_3D_VOLUME, R"(\$Nodes)", "Gmsh nodes section"},
            {FileType::MESH_3D_VOLUME, R"(\$Elements)", "Gmsh elements section"},

            // Surface mesh data
            {FileType::MESH_3D_SURFACE, R"(OIL\s+FILM\s+THICKNESS)", "Oil film thickness"},
            {FileType::MESH_3D_SURFACE, R"(CONTACT\s+PRESSURE)", "Contact pressure"},
            {FileType::MESH_3D_SURFACE, R"(TEMPERATURE\s+DISTRIBUTION)", "Temperature distribution"},
            {FileType::MESH_3D_SURFACE, R"(STRESS\s+DISTRIBUTION)", "Stress distribution"},
            {FileType::MESH_3D_SURFACE, R"(FILM\s+THICKNESS)", "Film thickness"},
            {FileType::MESH_3D_SURFACE, R"(VON-MISES STRESS)", "Von-Mises stress"},

            // Wear data
            {FileType::RING_WEAR, R"(CUMULATIVE\s+RING\s+FACE\s+WEAR)", "Ring face wear"},
            {FileType::RING_WEAR, R"(RING\s+BOTTOM\s+SIDE\s+WEAR)", "Ring bottom side wear"},
            {FileType::RING_WEAR, R"(RING\s+TOP\s+SIDE\s+WEAR)", "Ring top side wear"},
            {FileType::RING_WEAR, R"(WEAR AT TIME)", "Wear over time"},

            // Time series data
            {FileType::TIME_SERIES_2D, R"(TIME.*POSITION)", "Position vs time"},
            {FileType::TIME_SERIES_2D, R"(TIME.*VELOCITY)", "Velocity vs time"},
            {FileType::TIME_SERIES_2D, R"(TIME.*ACCELERATION)", "Acceleration vs time"},
            {FileType::TIME_SERIES_2D, R"(CRANK_ANGLE)", "Crank angle data"},
            {FileType::TIME_SERIES_2D, R"(PISTON\s+POSITION)", "Piston position"},
            {FileType::TIME_SERIES_2D, R"(PISTON\s+VELOCITY)", "Piston velocity"},
            {FileType::TIME_SERIES_2D, R"(PISTON\s+ACCELERATION)", "Piston acceleration"},

            // Correlation data
            {FileType::CORRELATION_2D, R"(POSITION.*VELOCITY)", "Position-velocity correlation"}
        };

        for (const auto& pattern : patterns) {
            try {
                std::regex re(pattern.regex_pattern, std::regex::icase);
                if (std::regex_search(content, re)) {
                    log("  Detected by pattern: " + pattern.description);
                    return pattern.type;
                }
            }
            catch (const std::regex_error&) {
                // Ignore regex errors
            }
        }

        // ===== STEP 2: Header analysis for structured data =====
        auto header_analysis = analyze_headers(lines);

        if (header_analysis.has_wear_keyword && header_analysis.has_grid_structure) {
            log("  Detected as WEAR data with grid structure");
            return FileType::RING_WEAR;
        }

        if (header_analysis.has_grid_indicators && header_analysis.numeric_density > 0.7) {
            log("  Detected as GRID_2D (matrix/grid structure)");
            return FileType::GRID_2D;
        }

        // ===== STEP 3: Numeric structure detection =====
        std::vector<std::vector<double>> numeric_lines;
        for (const auto& l : lines) {
            std::vector<double> numbers = extract_numbers_from_line(l);
            if (!numbers.empty()) {
                numeric_lines.push_back(numbers);
                if (numeric_lines.size() >= 100) break;
            }
        }

        if (numeric_lines.empty()) {
            log("  No numeric data found, type: UNKNOWN");
            return FileType::UNKNOWN;
        }

        int rows = static_cast<int>(numeric_lines.size());
        size_t first_cols = numeric_lines[0].size();

        // Check column consistency
        bool cols_consistent = true;
        for (int i = 1; i < std::min(20, rows); i++) {
            if (numeric_lines[i].size() != first_cols) {
                cols_consistent = false;
                break;
            }
        }

        // Classification based on shape
        if (cols_consistent && rows > 5 && first_cols > 5) {
            log("  Detected as GRID_2D (consistent " + std::to_string(first_cols) + " columns, " +
                std::to_string(rows) + " rows)");
            return FileType::GRID_2D;
        }
        else if (first_cols == 2) {
            bool increasing = true;
            for (int i = 1; i < std::min(20, rows); i++) {
                if (numeric_lines[i][0] <= numeric_lines[i - 1][0]) {
                    increasing = false;
                    break;
                }
            }
            FileType type = increasing ? FileType::TIME_SERIES_2D : FileType::CORRELATION_2D;
            log("  Detected as " + std::string(increasing ? "TIME_SERIES_2D" : "CORRELATION_2D"));
            return type;
        }
        else if (first_cols == 1) {
            log("  Detected as TIME_SERIES_2D (single column)");
            return FileType::TIME_SERIES_2D;
        }
        else if (first_cols == 3 && rows > 10) {
            log("  Detected as MESH_3D_SURFACE (3D point cloud)");
            return FileType::MESH_3D_SURFACE;
        }
        else {
            log("  Detected as GRID_2D (fallback)");
            return FileType::GRID_2D;
        }
    }

private:
    struct HeaderAnalysis {
        bool has_wear_keyword = false;
        bool has_grid_indicators = false;
        bool has_radial_nodes = false;
        bool has_matrix_format = false;
        bool has_grid_structure = false;
        double numeric_density = 0.0;
        int header_lines = 0;
        std::vector<std::string> keywords_found;
    };

    static HeaderAnalysis analyze_headers(const std::vector<std::string>& lines) {
        HeaderAnalysis analysis;

        std::vector<std::string> wear_keywords = { "WEAR", "CUMULATIVE", "RING", "BOTTOM", "TOP", "FACE" };
        std::vector<std::string> grid_keywords = { "RADIAL", "CIRCUMFERENTIAL", "NODE", "GRID", "MATRIX" };
        std::vector<std::string> matrix_indicators = { "ROW", "COLUMN", "I, J", "(I,J)" };

        int text_lines = 0;
        int total_lines_checked = 0;

        for (size_t i = 0; i < std::min(lines.size(), size_t(30)); i++) {
            const auto& line = lines[i];
            if (line.empty()) continue;

            std::string upper = line;
            std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
            total_lines_checked++;

            // Check for wear keywords
            for (const auto& kw : wear_keywords) {
                if (upper.find(kw) != std::string::npos) {
                    analysis.has_wear_keyword = true;
                    analysis.keywords_found.push_back(kw);
                }
            }

            // Check for grid indicators
            for (const auto& kw : grid_keywords) {
                if (upper.find(kw) != std::string::npos) {
                    analysis.has_grid_indicators = true;
                }
            }

            // Check for matrix format indicators
            for (const auto& ind : matrix_indicators) {
                if (upper.find(ind) != std::string::npos) {
                    analysis.has_matrix_format = true;
                }
            }

            // Check for radial nodes specification
            if (upper.find("RADIAL NODES") != std::string::npos) {
                analysis.has_radial_nodes = true;
                analysis.has_grid_structure = true;
            }

            // Check if line is mostly text (header)
            bool is_text_line = false;
            int alpha_count = 0;
            int total_chars = 0;
            for (char c : line) {
                if (std::isalpha(static_cast<unsigned char>(c))) alpha_count++;
                if (!std::isspace(static_cast<unsigned char>(c))) total_chars++;
            }
            if (total_chars > 0 && (static_cast<double>(alpha_count) / total_chars) > 0.3) {
                is_text_line = true;
                text_lines++;
            }
        }

        analysis.header_lines = text_lines;
        analysis.numeric_density = 1.0 - (static_cast<double>(text_lines) / std::max(1, total_lines_checked));

        // Determine grid structure
        if (analysis.has_radial_nodes || (analysis.has_matrix_format && analysis.has_wear_keyword)) {
            analysis.has_grid_structure = true;
        }

        return analysis;
    }

    static std::vector<double> extract_numbers_from_line(const std::string& line) {
        std::vector<double> numbers;
        std::regex num_re(R"([-+]?\d*\.?\d+(?:[eEdD][-+]?\d+)?)");

        auto words_begin = std::sregex_iterator(line.begin(), line.end(), num_re);
        auto words_end = std::sregex_iterator();

        for (auto it = words_begin; it != words_end; ++it) {
            try {
                std::string num_str = it->str();
                // Replace D/d with e for scientific notation
                for (char& c : num_str) {
                    if (c == 'D' || c == 'd') c = 'e';
                }
                numbers.push_back(std::stod(num_str));
            }
            catch (...) {
                continue;
            }
        }

        return numbers;
    }
};

#endif // FILE_DETECTOR_H