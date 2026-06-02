#ifndef JSON_BUILDER_H
#define JSON_BUILDER_H

#include <sstream>
#include <iomanip>
#include <functional>

#include "../config/config.h"

class JsonBuilder
{
public:

    static std::string build(
        const ParsedData& data,
        std::function<void(const std::string&)> log = nullptr)
    {
        std::ostringstream json;

        json << std::fixed
            << std::setprecision(8);

        json << "{\n";

        write_metadata(json, data);

        json << ",\n";

        write_plot(json, data);

        json << ",\n";

        write_layout(json, data);

        json << "\n}";

        return json.str();
    }

private:

    // =========================================================
    // METADATA
    // =========================================================

    static void write_metadata(
        std::ostringstream& json,
        const ParsedData& data)
    {
        json << "\"metadata\": {\n";

        json << "\"filename\": \""
            << data.filename
            << "\",\n";

        json << "\"plot_type\": \""
            << data.plot_type
            << "\",\n";

        json << "\"data_type\": \""
            << data.data_type
            << "\"";

        for (const auto& meta : data.metadata)
        {
            if (meta.first == "plot_type")
                continue;

            if (meta.first == "data_type")
                continue;

            json << ",\n";

            json << "\""
                << meta.first
                << "\": \""
                << meta.second
                << "\"";
        }

        json << "\n}";
    }

    // =========================================================
    // DATA
    // =========================================================

    static void write_plot(
        std::ostringstream& json,
        const ParsedData& data)
    {
        json << "\"data\": [\n";

        if (data.plot_type == "surface")
        {
            write_surface(json, data);
        }
        else if (data.plot_type == "heatmap")
        {
            write_heatmap(json, data);
        }
        else if (data.plot_type == "scatter")
        {
            write_scatter(json, data);
        }
        else if (data.plot_type == "mesh3d")
        {
            write_mesh(json, data);
        }

        json << "\n]";
    }

    // =========================================================
    // LAYOUT
    // =========================================================

    static void write_layout(
        std::ostringstream& json,
        const ParsedData& data)
    {
        json << "\"layout\": {";

        // =====================================================
        // TITLE
        // =====================================================

        std::string title;

        auto titleIt =
            data.metadata.find("title");

        if (titleIt != data.metadata.end())
        {
            title = titleIt->second;
        }
        else if (data.data_type == "stress")
        {
            title = "Von-Mises Stress Distribution";
        }
        else if (data.data_type == "temperature")
        {
            title = "Temperature Distribution";
        }
        else if (data.data_type == "wear")
        {
            title = "Wear Distribution";
        }
        else if (data.data_type == "pressure")
        {
            title = "Pressure Distribution";
        }
        else
        {
            title = data.filename;
        }

        json << "\"title\": \"" << title << "\"";

        // =====================================================
        // AXIS LABELS
        // =====================================================

        std::string xTitle = "X";
        std::string yTitle = "Y";

        auto xAxisIt =
            data.metadata.find("x_axis");

        auto yAxisIt =
            data.metadata.find("y_axis");

        if (xAxisIt != data.metadata.end())
        {
            xTitle = xAxisIt->second;
        }

        if (yAxisIt != data.metadata.end())
        {
            yTitle = yAxisIt->second;
        }


        // =====================================================
        // Z LABEL
        // =====================================================

        std::string zTitle = "Value";

        auto units_it =
            data.metadata.find("units");

        std::string units;

        if (units_it != data.metadata.end())
        {
            units = units_it->second;
        }

        if (data.data_type == "stress")
        {
            zTitle = "Stress";
        }
        else if (data.data_type == "temperature")
        {
            zTitle = "Temperature";
        }
        else if (data.data_type == "wear")
        {
            zTitle = "Wear Depth";
        }
        else if (data.data_type == "pressure")
        {
            zTitle = "Pressure";
        }

        if (!units.empty())
        {
            zTitle += " (" + units + ")";
        }

        // =====================================================
        // SURFACE / MESH3D
        // =====================================================

        if (data.plot_type == "surface" ||
            data.plot_type == "mesh3d")
        {
            json << ",";

            json << "\"scene\": {";

            json << "\"xaxis\": {";
            json << "\"title\": \"" << xTitle << "\"";
            json << "},";

            json << "\"yaxis\": {";
            json << "\"title\": \"" << yTitle << "\"";
            json << "},";

            json << "\"zaxis\": {";
            json << "\"title\": \"" << zTitle << "\"";
            json << "}";

            json << "}";
        }

        // =====================================================
        // HEATMAP
        // =====================================================

        else if (data.plot_type == "heatmap")
        {
            json << ",";

            json << "\"xaxis\": {";
            json << "\"title\": \"" << xTitle << "\"";
            json << "},";

            json << "\"yaxis\": {";
            json << "\"title\": \"" << yTitle << "\"";
            json << "}";
        }

        // =====================================================
        // SCATTER
        // =====================================================

        else if (data.plot_type == "scatter")
        {
            json << ",";

            json << "\"xaxis\": {";
            json << "\"title\": \"" << xTitle << "\"";
            json << "},";

            json << "\"yaxis\": {";
            json << "\"title\": \"" << yTitle << "\"";
            json << "}";
        }

        json << "}";
    }

    // =========================================================
    // SURFACE
    // =========================================================

    static void write_surface(
        std::ostringstream& json,
        const ParsedData& data)
    {
        if (data.numeric_data.empty())
        {
            json << "{}";
            return;
        }

        json << "{";

        json << "\"type\":\"surface\",";
        json << "\"colorscale\":\"Viridis\",";
        json << "\"showscale\":true,";

        json << "\"z\":[";

        for (size_t r = 0;
            r < data.numeric_data.size();
            r++)
        {
            if (r) json << ",";

            json << "[";

            for (size_t c = 0;
                c < data.numeric_data[r].size();
                c++)
            {
                if (c) json << ",";

                json << data.numeric_data[r][c];
            }

            json << "]";
        }

        json << "]";

        json << "}";
    }

    // =========================================================
    // HEATMAP
    // =========================================================

    static void write_heatmap(
        std::ostringstream& json,
        const ParsedData& data)
    {
        if (data.numeric_data.empty())
        {
            json << "{}";
            return;
        }

        json << "{";

        json << "\"type\":\"heatmap\",";
        json << "\"colorscale\":\"Viridis\",";
        json << "\"showscale\":true,";

        json << "\"z\":[";

        for (size_t r = 0;
            r < data.numeric_data.size();
            r++)
        {
            if (r) json << ",";

            json << "[";

            for (size_t c = 0;
                c < data.numeric_data[r].size();
                c++)
            {
                if (c) json << ",";

                json << data.numeric_data[r][c];
            }

            json << "]";
        }

        json << "]";

        json << "}";
    }

    // =========================================================
    // SCATTER
    // =========================================================

    static void write_scatter(
        std::ostringstream& json,
        const ParsedData& data)
    {
        json << "{";

        json << "\"type\":\"scatter\",";
        json << "\"mode\":\"lines\",";

        // ===== X =====

        json << "\"x\":[";

        bool first = true;

        for (size_t i = 0;
            i < data.numeric_data.size();
            i++)
        {
            if (data.numeric_data[i].size() < 2)
            {
                continue;
            }

            if (!first) json << ",";

            json << data.numeric_data[i][0];

            first = false;
        }

        json << "],";

        // ===== Y =====

        json << "\"y\":[";

        first = true;

        for (size_t i = 0;
            i < data.numeric_data.size();
            i++)
        {
            if (data.numeric_data[i].size() < 2)
            {
                continue;
            }

            if (!first) json << ",";

            json << data.numeric_data[i][1];

            first = false;
        }

        json << "]";

        json << "}";
    }

    // =========================================================
    // MESH3D
    // =========================================================

    static void write_mesh(
        std::ostringstream& json,
        const ParsedData& data)
    {
        json << "{";

        json << "\"type\":\"mesh3d\",";
        json << "\"colorscale\":\"Viridis\",";
        json << "\"showscale\":true,";

        // ===== X =====

        json << "\"x\":[";

        for (size_t i = 0;
            i < data.mesh.x.size();
            i++)
        {
            if (i) json << ",";

            json << data.mesh.x[i];
        }

        json << "],";

        // ===== Y =====

        json << "\"y\":[";

        for (size_t i = 0;
            i < data.mesh.y.size();
            i++)
        {
            if (i) json << ",";

            json << data.mesh.y[i];
        }

        json << "],";

        // ===== Z =====

        json << "\"z\":[";

        for (size_t i = 0;
            i < data.mesh.z.size();
            i++)
        {
            if (i) json << ",";

            json << data.mesh.z[i];
        }

        json << "],";

        // ===== I =====

        json << "\"i\":[";

        for (size_t i = 0;
            i < data.mesh.i.size();
            i++)
        {
            if (i) json << ",";

            json << data.mesh.i[i];
        }

        json << "],";

        // ===== J =====

        json << "\"j\":[";

        for (size_t i = 0;
            i < data.mesh.j.size();
            i++)
        {
            if (i) json << ",";

            json << data.mesh.j[i];
        }

        json << "],";

        // ===== K =====

        json << "\"k\":[";

        for (size_t i = 0;
            i < data.mesh.k.size();
            i++)
        {
            if (i) json << ",";

            json << data.mesh.k[i];
        }

        json << "]";

        // ===== INTENSITY =====

        if (!data.mesh.intensity.empty())
        {
            json << ",\"intensity\":[";

            for (size_t i = 0;
                i < data.mesh.intensity.size();
                i++)
            {
                if (i) json << ",";

                json << data.mesh.intensity[i];
            }

            json << "]";
        }

        json << "}";
    }
};

#endif // JSON_BUILDER_H