#ifndef FILE_DETECTOR_H
#define FILE_DETECTOR_H

#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>
#include <vector>
#include <functional>

#include "inspection_result.h"

class FileInspector
{
public:

    static InspectionResult inspect(
        const std::string& filepath,
        std::function<void(const std::string&)> log = nullptr)
    {
        InspectionResult result;

        std::ifstream file(filepath);

        if (!file.is_open())
        {
            return result;
        }

        std::string line;
        int line_index = 0;

        // Read up to first 50 lines (safe guard)
        while (std::getline(file, line) && line_index < 50)
        {
            std::string upper = to_upper(line);

            // ===== SURFACE =====
            if (upper.find("VON-MISES STRESS") != std::string::npos ||
                upper.find("VON MISES") != std::string::npos)
            {
                result.type = FileType::MESH_3D_SURFACE;
                result.data_type = "stress_surface";
                result.metadata["title"] = trim(line);
                result.metadata["plot_type"] = "surface";
            }
            // ===== WEAR =====
            else if (upper.find("WEAR") != std::string::npos)
            {
                result.type = FileType::RING_WEAR;
                result.data_type = "wear_surface";
                result.metadata["plot_type"] = "surface";
            }
            // ===== 2D detection (simple heuristics) =====
            else if (upper.find("TIME") != std::string::npos && upper.find("SERIES") != std::string::npos)
            {
                result.type = FileType::TIME_SERIES_2D;
                result.data_type = "time_series";
                result.metadata["plot_type"] = "scatter";
            }
            else if (upper.find("GRID") != std::string::npos || upper.find("MESH") != std::string::npos)
            {
                result.type = FileType::GRID_2D;
                result.data_type = "grid";
                result.metadata["plot_type"] = "heatmap";
            }

            // store metadata lines for later use
            if (!trim(line).empty())
            {
                result.metadata_lines.push_back(line);
            }

            line_index++;
        }

        // minimal header heuristic: if first non-empty line contains non-numeric content, count it as header_lines = number of metadata lines
        int headers = 0;
        for (const auto& l : result.metadata_lines)
        {
            // if line contains any alpha char, treat as header/meta
            bool has_alpha = false;
            for (char c : l)
            {
                if (std::isalpha(static_cast<unsigned char>(c)))
                {
                    has_alpha = true;
                    break;
                }
            }
            if (has_alpha)
            {
                headers++;
            }
        }
        result.header_lines = headers;

        // if plot_type metadata not set ensure a default
        if (result.metadata.find("plot_type") != result.metadata.end())
        {
            result.data_type = result.data_type.empty() ? result.metadata["plot_type"] : result.data_type;
        }

        return result;
    }

private:
    static std::string to_upper(const std::string& s)
    {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(), ::toupper);
        return out;
    }

    static std::string trim(const std::string& s)
    {
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }
};

#endif // FILE_DETECTOR_H