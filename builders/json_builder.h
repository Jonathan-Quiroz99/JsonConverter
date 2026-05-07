#ifndef JSON_BUILDER_H
#define JSON_BUILDER_H

#include <string>
#include <sstream>
#include <functional>
#include <vector>
#include <map>
#include <iomanip>
#include <cctype>
#include "../config/config.h"

class JsonBuilder {
public:
    static std::string build(const ParsedData& data,
        std::function<void(const std::string&)> log_callback = nullptr) {

        auto log = [&](const std::string& msg) {
            if (log_callback) log_callback(msg);
            };

        std::ostringstream json;
        json << std::fixed << std::setprecision(8);

        auto escape_json = [&](const std::string& s) -> std::string {
            std::string result;
            for (char c : s) {
                switch (c) {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\b': result += "\\b"; break;
                case '\f': result += "\\f"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += c;
                }
            }
            return result;
            };

        auto array_to_json_num = [&](const auto& arr) -> std::string {
            std::ostringstream oss;
            oss << "[";
            for (size_t i = 0; i < arr.size(); ++i) {
                if (i) oss << ", ";
                oss << arr[i];
            }
            oss << "]";
            return oss.str();
            };

        auto format_matrix_as_heatmap = [&](const std::vector<std::vector<double>>& matrix,
            const std::string& title) -> std::string {
                std::ostringstream oss;
                oss << "{\"type\":\"heatmap\",\"name\":\"" << escape_json(title) << "\",\"z\":[";
                size_t max_rows = std::min(matrix.size(), size_t(1000));
                for (size_t r = 0; r < max_rows; ++r) {
                    if (r) oss << ",";
                    oss << "[";
                    size_t max_cols = std::min(matrix[r].size(), size_t(1000));
                    for (size_t c = 0; c < max_cols; ++c) {
                        if (c) oss << ",";
                        oss << matrix[r][c];
                    }
                    oss << "]";
                }
                oss << "]}";
                return oss.str();
            };

        auto format_matrix_as_surface = [&](const std::vector<std::vector<double>>& matrix,
            const std::string& title) -> std::string {
                std::ostringstream oss;
                oss << "{\"type\":\"surface\",\"name\":\"" << escape_json(title) << "\",\"z\":[";
                size_t max_rows = std::min(matrix.size(), size_t(500));
                for (size_t r = 0; r < max_rows; ++r) {
                    if (r) oss << ",";
                    oss << "[";
                    size_t max_cols = std::min(matrix[r].size(), size_t(500));
                    for (size_t c = 0; c < max_cols; ++c) {
                        if (c) oss << ",";
                        oss << matrix[r][c];
                    }
                    oss << "]";
                }
                oss << "]}";
                return oss.str();
            };

        auto format_matrix_as_contour = [&](const std::vector<std::vector<double>>& matrix,
            const std::string& title) -> std::string {
                std::ostringstream oss;
                oss << "{\"type\":\"contour\",\"name\":\"" << escape_json(title) << "\",\"z\":[";
                size_t max_rows = std::min(matrix.size(), size_t(500));
                for (size_t r = 0; r < max_rows; ++r) {
                    if (r) oss << ",";
                    oss << "[";
                    size_t max_cols = std::min(matrix[r].size(), size_t(500));
                    for (size_t c = 0; c < max_cols; ++c) {
                        if (c) oss << ",";
                        oss << matrix[r][c];
                    }
                    oss << "]";
                }
                oss << "]}";
                return oss.str();
            };

        // Begin Plotly structure
        json << "{\n";
        json << "  \"filename\": \"" << escape_json(data.filename) << "\",\n";
        json << "  \"type\": " << static_cast<int>(data.type) << ",\n";
        json << "  \"data_type\": \"" << escape_json(data.data_type) << "\",\n";
        json << "  \"shape\": {\n";
        json << "    \"rows\": " << data.shape.rows << ",\n";
        json << "    \"cols\": " << data.shape.cols << ",\n";
        json << "    \"is_matrix\": " << (data.shape.is_matrix ? "true" : "false") << ",\n";
        json << "    \"layout\": \"" << escape_json(data.shape.layout) << "\"\n";
        json << "  },\n";
        json << "  \"data\": [\n";

        bool first_trace = true;

        // ===== MATRIX DATA (Heatmap + Surface + Contour) =====
        if (data.is_matrix_data() && !data.numeric_data.empty()) {
            size_t rows = data.numeric_data.size();
            size_t cols = data.numeric_data[0].size();

            log("  Generating visualizations for matrix data (" + std::to_string(rows) + "x" + std::to_string(cols) + ")");

            // Heatmap (always included)
            if (!first_trace) json << ",\n";
            first_trace = false;
            json << "    " << format_matrix_as_heatmap(data.numeric_data, data.data_type + "_heatmap");

            // Surface plot (for smaller matrices or when appropriate)
            if (rows <= 200 && cols <= 200 && rows > 2 && cols > 2) {
                json << ",\n";
                json << "    " << format_matrix_as_surface(data.numeric_data, data.data_type + "_surface");
            }

            // Contour plot (for medium-sized matrices)
            if (rows <= 500 && cols <= 500 && rows > 3 && cols > 3) {
                json << ",\n";
                json << "    " << format_matrix_as_contour(data.numeric_data, data.data_type + "_contour");
            }
        }
        // ===== NUMERIC DATA (non-matrix: time series, scatter, etc.) =====
        else if (!data.numeric_data.empty()) {
            size_t rows = data.numeric_data.size();
            size_t cols = data.numeric_data[0].size();

            // check consistent row lengths
            bool consistent = true;
            for (size_t r = 1; r < rows; ++r) {
                if (data.numeric_data[r].size() != cols) { consistent = false; break; }
            }

            if (cols == 2) {
                // scatter
                std::vector<double> x(rows), y(rows);
                for (size_t r = 0; r < rows; ++r) {
                    x[r] = data.numeric_data[r][0];
                    y[r] = data.numeric_data[r][1];
                }
                if (!first_trace) json << ",\n";
                first_trace = false;
                json << "    {\"type\":\"scatter\",\"mode\":\"lines+markers\",\"name\":\""
                    << escape_json(data.data_type.empty() ? "Series" : data.data_type)
                    << "\",\"x\":" << array_to_json_num(x)
                    << ",\"y\":" << array_to_json_num(y) << "}";
            }
            else if (cols == 3) {
                // scatter3d
                std::vector<double> x(rows), y(rows), z(rows);
                for (size_t r = 0; r < rows; ++r) {
                    x[r] = data.numeric_data[r][0];
                    y[r] = data.numeric_data[r][1];
                    z[r] = data.numeric_data[r][2];
                }
                if (!first_trace) json << ",\n";
                first_trace = false;
                json << "    {\"type\":\"scatter3d\",\"mode\":\"lines+markers\",\"name\":\""
                    << escape_json(data.data_type.empty() ? "3D Series" : data.data_type)
                    << "\",\"x\":" << array_to_json_num(x)
                    << ",\"y\":" << array_to_json_num(y)
                    << ",\"z\":" << array_to_json_num(z) << "}";
            }
            else if (cols == 1) {
                // single column - use index as X
                std::vector<double> x(rows), y(rows);
                for (size_t r = 0; r < rows; ++r) {
                    x[r] = static_cast<double>(r);
                    y[r] = data.numeric_data[r][0];
                }
                if (!first_trace) json << ",\n";
                first_trace = false;
                json << "    {\"type\":\"scatter\",\"mode\":\"lines+markers\",\"name\":\""
                    << escape_json(data.data_type.empty() ? "Series" : data.data_type)
                    << "\",\"x\":" << array_to_json_num(x)
                    << ",\"y\":" << array_to_json_num(y) << "}";
            }
            else {
                // multiple Y series, first col is x
                std::vector<double> x(rows);
                for (size_t r = 0; r < rows; ++r) x[r] = data.numeric_data[r][0];

                for (size_t c = 1; c < cols; ++c) {
                    std::vector<double> y(rows);
                    for (size_t r = 0; r < rows; ++r) {
                        y[r] = (data.numeric_data[r].size() > c) ? data.numeric_data[r][c] : 0.0;
                    }
                    if (!first_trace) json << ",\n";
                    first_trace = false;
                    std::string name = data.headers.size() > c ? data.headers[c] : ("Series " + std::to_string(c));
                    json << "    {\"type\":\"scatter\",\"mode\":\"lines+markers\",\"name\":\""
                        << escape_json(name) << "\",\"x\":" << array_to_json_num(x)
                        << ",\"y\":" << array_to_json_num(y) << "}";
                }

                // Heatmap for matrix-like data (consistent columns)
                if (consistent && rows > 1 && cols > 1 && rows * cols <= 10000) {
                    if (!first_trace) json << ",\n";
                    first_trace = false;
                    json << "    {\"type\":\"heatmap\",\"name\":\"Heatmap\",\"z\":[";
                    for (size_t r = 0; r < rows; ++r) {
                        if (r) json << ",";
                        json << "[";
                        for (size_t c = 0; c < cols; ++c) {
                            if (c) json << ",";
                            json << data.numeric_data[r][c];
                        }
                        json << "]";
                    }
                    json << "]}";
                }
            }
        }

        // ===== MESH DATA =====
        if (data.has_mesh() && !data.mesh.x.empty()) {
            if (!first_trace) json << ",\n";
            first_trace = false;

            json << "    {\"type\":\"mesh3d\",\"name\":\"3D Mesh\"";
            json << ",\"x\":" << array_to_json_num(data.mesh.x);
            json << ",\"y\":" << array_to_json_num(data.mesh.y);
            json << ",\"z\":" << array_to_json_num(data.mesh.z);

            if (!data.mesh.i.empty() && !data.mesh.j.empty() && !data.mesh.k.empty()) {
                json << ",\"i\":" << array_to_json_num(data.mesh.i);
                json << ",\"j\":" << array_to_json_num(data.mesh.j);
                json << ",\"k\":" << array_to_json_num(data.mesh.k);
            }

            if (!data.mesh.intensity.empty()) {
                json << ",\"intensity\":" << array_to_json_num(data.mesh.intensity);
                json << ",\"colorscale\":\"Viridis\"";
            }
            json << "}";
        }

        // ===== SECTIONS DATA =====
        if (data.has_sections()) {
            for (const auto& sec_pair : data.sections) {
                for (const auto& comp_pair : sec_pair.second.components) {
                    if (comp_pair.second.empty()) continue;

                    if (!first_trace) json << ",\n";
                    first_trace = false;

                    std::string trace_name = sec_pair.first + "_comp_" + std::to_string(comp_pair.first);
                    size_t comp_rows = comp_pair.second.size();
                    size_t comp_cols = comp_pair.second[0].size();

                    // Check if this section data is a matrix
                    bool is_matrix = true;
                    for (size_t i = 1; i < std::min(comp_rows, size_t(10)); i++) {
                        if (comp_pair.second[i].size() != comp_cols) {
                            is_matrix = false;
                            break;
                        }
                    }

                    if (is_matrix && comp_rows > 2 && comp_cols > 2 && comp_rows * comp_cols <= 5000) {
                        // Render as heatmap
                        json << "    {\"type\":\"heatmap\",\"name\":\"" << escape_json(trace_name) << "\",\"z\":[";
                        for (size_t r = 0; r < comp_rows && r < 200; ++r) {
                            if (r) json << ",";
                            json << "[";
                            for (size_t c = 0; c < comp_cols && c < 200; ++c) {
                                if (c) json << ",";
                                json << comp_pair.second[r][c];
                            }
                            json << "]";
                        }
                        json << "]}";
                    }
                    else if (comp_cols == 3) {
                        // 3D scatter
                        json << "    {\"type\":\"scatter3d\",\"mode\":\"markers\",\"name\":\""
                            << escape_json(trace_name) << "\",\"x\":[";
                        for (size_t r = 0; r < comp_rows; ++r) {
                            if (r) json << ",";
                            json << comp_pair.second[r][0];
                        }
                        json << "],\"y\":[";
                        for (size_t r = 0; r < comp_rows; ++r) {
                            if (r) json << ",";
                            json << comp_pair.second[r][1];
                        }
                        json << "],\"z\":[";
                        for (size_t r = 0; r < comp_rows; ++r) {
                            if (r) json << ",";
                            json << comp_pair.second[r][2];
                        }
                        json << "]}";
                    }
                    else if (comp_cols == 2) {
                        // 2D scatter
                        json << "    {\"type\":\"scatter\",\"mode\":\"lines+markers\",\"name\":\""
                            << escape_json(trace_name) << "\",\"x\":[";
                        for (size_t r = 0; r < comp_rows; ++r) {
                            if (r) json << ",";
                            json << comp_pair.second[r][0];
                        }
                        json << "],\"y\":[";
                        for (size_t r = 0; r < comp_rows; ++r) {
                            if (r) json << ",";
                            json << comp_pair.second[r][1];
                        }
                        json << "]}";
                    }
                    else if (comp_cols == 1) {
                        // Single column - use index as X
                        json << "    {\"type\":\"scatter\",\"mode\":\"lines+markers\",\"name\":\""
                            << escape_json(trace_name) << "\",\"x\":[";
                        for (size_t r = 0; r < comp_rows; ++r) {
                            if (r) json << ",";
                            json << r;
                        }
                        json << "],\"y\":[";
                        for (size_t r = 0; r < comp_rows; ++r) {
                            if (r) json << ",";
                            json << comp_pair.second[r][0];
                        }
                        json << "]}";
                    }
                }
            }
        }

        // Close data array
        json << "\n  ],\n";

        // ===== STATISTICS (for matrix data) =====
        if (data.is_matrix_data() && !data.numeric_data.empty()) {
            json << "  \"statistics\": {\n";
            json << "    \"min_value\": " << data.metadata.count("min_value") << ",\n";
            json << "    \"max_value\": " << data.metadata.count("max_value") << ",\n";
            json << "    \"total_elements\": " << (data.shape.rows * data.shape.cols) << "\n";
            json << "  },\n";
        }

        // ===== METADATA =====
        json << "  \"metadata\": {\n";
        bool first_meta = true;
        for (const auto& meta : data.metadata) {
            if (!first_meta) json << ",\n";
            first_meta = false;
            json << "    \"" << escape_json(meta.first) << "\": \"" << escape_json(meta.second) << "\"";
        }
        json << "\n  }\n";

        // Close main object
        json << "}\n";

        return json.str();
    }

private:
    // Helper method to format matrix as heatmap (exposed for internal use)
    static std::string format_matrix_as_heatmap(const std::vector<std::vector<double>>& matrix,
        const std::string& title) {
        std::ostringstream oss;
        oss << "{\"type\":\"heatmap\",\"name\":\"" << title << "\",\"z\":[";
        size_t max_rows = std::min(matrix.size(), size_t(1000));
        for (size_t r = 0; r < max_rows; ++r) {
            if (r) oss << ",";
            oss << "[";
            size_t max_cols = std::min(matrix[r].size(), size_t(1000));
            for (size_t c = 0; c < max_cols; ++c) {
                if (c) oss << ",";
                oss << matrix[r][c];
            }
            oss << "]";
        }
        oss << "]}";
        return oss.str();
    }
};

#endif // JSON_BUILDER_H