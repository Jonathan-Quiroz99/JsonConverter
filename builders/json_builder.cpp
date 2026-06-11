#include "json_builder.h"

#include <sstream>
#include <iomanip>

std::string JsonBuilder::build(
    const PlotData& data)
{
    switch (data.plotType)
    {
    case PlotType::Scatter:
        return buildScatter(data);

    case PlotType::Surface:
        return buildSurface(data);

	case PlotType::Heatmap:
		return buildHeatmap(data);

    case PlotType::Scatter3D:
        return buildScatter3D(data);

    default:
        return "{}";
    }
}

std::string JsonBuilder::buildSurface(
    const PlotData& data)
{
    if (!data.frames.empty())
    {
        return buildAnimatedSurface(data);
    }

    return buildStaticSurface(data);
}

std::string JsonBuilder::buildScatter(
    const PlotData& data)
{
    std::ostringstream json;

    json << std::fixed
        << std::setprecision(6);

    json << "{\n";

    json << "\"data\":[\n";

    for (size_t s = 0;
        s < data.series.size();
        s++)
    {
        const auto& series =
            data.series[s];

        if (s > 0)
        {
            json << ",";
        }

        json << "{";

        json << "\"type\":\"scatter\",";
        json << "\"mode\":\"lines\",";
        json << "\"name\":\""
            << series.name
            << "\",";

        json << "\"x\":[";

        for (size_t i = 0;
            i < series.x.size();
            i++)
        {
            if (i) json << ",";

            json << series.x[i];
        }

        json << "],";

        json << "\"y\":[";

        for (size_t i = 0;
            i < series.y.size();
            i++)
        {
            if (i) json << ",";

            json << series.y[i];
        }

        json << "]";

        json << "}";
    }

    json << "],";

    json << "\"layout\":{";

    // ======================
    // TITLE
    // ======================

    json << "\"title\":{";
    json << "\"text\":\""
        << data.title
        << "\"";
    json << "},";

    // ======================
    // X AXIS
    // ======================

    json << "\"xaxis\":{";

    json << "\"title\":{";
    json << "\"text\":\""
        << data.xLabel
        << "\"";
    json << "}";

    json << "},";

    // ======================
    // Y AXIS
    // ======================

    json << "\"yaxis\":{";

    json << "\"title\":{";
    json << "\"text\":\""
        << data.yLabel
        << "\"";
    json << "}";

    json << "}";

    json << "}";

    json << "}";

    return json.str();
}

std::string JsonBuilder::buildStaticSurface(
    const PlotData& data)
{
    std::ostringstream json;

    json << std::fixed
        << std::setprecision(6);

    json << "{";

    json << "\"data\":[";

    json << "{";

    json << "\"type\":\"surface\",";
    json << "\"colorscale\":\"Viridis\",";

    json << "\"z\":[";

    for (size_t r = 0;
        r < data.matrix.size();
        r++)
    {
        if (r)
        {
            json << ",";
        }

        json << "[";

        for (size_t c = 0;
            c < data.matrix[r].size();
            c++)
        {
            if (c)
            {
                json << ",";
            }

            json << data.matrix[r][c];
        }

        json << "]";
    }

    json << "]";

    json << "}";

    json << "],";

    json << "\"layout\":{";

    // ======================
    // TITLE
    // ======================

    json << "\"title\":{";
    json << "\"text\":\""
        << data.title
        << "\"";
    json << "},";

    // ======================
    // SCENE
    // ======================

    json << "\"scene\":{";

    // X

    json << "\"xaxis\":{";

    json << "\"title\":{";
    json << "\"text\":\""
        << data.xLabel
        << "\"";
    json << "}";

    json << "},";

    // Y

    json << "\"yaxis\":{";

    json << "\"title\":{";
    json << "\"text\":\""
        << data.yLabel
        << "\"";
    json << "}";

    json << "},";

    // Z

    json << "\"zaxis\":{";

    json << "\"title\":{";
    json << "\"text\":\""
        << data.zLabel
        << "\"";
    json << "}";

    json << "}";

    json << "}";

    json << "}";

    json << "}";

    return json.str();
}

std::string JsonBuilder::buildHeatmap(
    const PlotData& data)
{
    // For simplicity, we can reuse the surface plot structure
    // and just change the type to "heatmap".
    std::ostringstream json;
    json << std::fixed
        << std::setprecision(6);
    json << "{";
    json << "\"data\":[";
    json << "{";
    json << "\"type\":\"heatmap\",";
    json << "\"colorscale\":\"Viridis\",";
    json << "\"z\":[";
    for (size_t r = 0;
        r < data.matrix.size();
        r++)
    {
        if (r)
        {
            json << ",";
        }
        json << "[";
        for (size_t c = 0;
            c < data.matrix[r].size();
            c++)
        {
            if (c)
            {
                json << ",";
            }
            json << data.matrix[r][c];
        }
        json << "]";
    }
    json << "]";
    json << "}";
    json << "],";
    // Layout can be similar to surface plot
    json << "\"layout\":{";
    // ======================
    // TITLE
    // ======================
    json << "\"title\":{";
    json << "\"text\":\""
        << data.title
        << "\"";
    json << "},";
    // ======================
    // X AXIS
    // ======================
    json << "\"xaxis\":{";
    json << "\"title\":{";
    json << "\"text\":\""
        << data.xLabel
        << "\"";
    json << "}";
    json << "},";
    // ======================
    // Y AXIS
    // ======================
    json << "\"yaxis\":{";
    json << "\"title\":{";
    json << "\"text\":\""
        << data.yLabel
        << "\"";
    json << "}";
    json << "}";
    json << "}";
    json << "}";
    return json.str();
}

std::string JsonBuilder::buildScatter3D(
    const PlotData& data)
{
    std::ostringstream json;
    json << std::fixed
        << std::setprecision(6);
    json << "{\n";
    json << "\"data\":[\n";
    for (size_t s = 0;
        s < data.series.size();
        s++)
    {
        const auto& series =
            data.series[s];
        if (s > 0)
        {
            json << ",";
        }
        json << "{";
        json << "\"type\":\"scatter3d\",";
        json << "\"mode\":\"markers\",";
        json << "\"name\":\""
            << series.name
            << "\",";
        json << "\"x\":[";
        for (size_t i = 0;
            i < series.x.size();
            i++)
        {
            if (i) json << ",";
            json << series.x[i];
        }
        json << "],";
        json << "\"y\":[";
        for (size_t i = 0;
            i < series.y.size();
            i++)
        {
            if (i) json << ",";
            json << series.y[i];
        }
        json << "],";
        json << "\"z\":[";
        for (size_t i = 0;
            i < series.z.size();
            i++)
        {
            if (i) json << ",";
            json << series.z[i];
        }
        json << "]";
        json << "}";
    }
    // Layout can be similar to surface plot
    // with scene containing xaxis, yaxis, zaxis
    json << "],";
    json << "\"layout\":{";
    // ======================
    // TITLE
    // ======================
    json << "\"title\":{";
    json << "\"text\":\""
        << data.title
        << "\"";
    json << "},";
    // ======================
    // SCENE
    // ======================
    json << "\"scene\":{";
    // X
    json << "\"xaxis\":{";
    json << "\"title\":{";
    json << "\"text\":\""
        << data.xLabel
        << "\"";
    json << "}";
    json << "},";
    // Y
    json << "\"yaxis\":{";
    json << "\"title\":{";
	json << "\"text\":\""
        << data.yLabel
        << "\"";
    json << "}";
    json << "},";
    // Z
    json << "\"zaxis\":{";
    json << "\"title\":{";
    json << "\"text\":\""
        << data.zLabel
        << "\"";
    json << "}";
    json << "}";
    json << "}";
    json << "}";
    return json.str();
}

std::string JsonBuilder::buildAnimatedSurface(
    const PlotData& data)
{
    if (data.frames.empty())
    {
        return "{}";
    }

    std::ostringstream json;

    json << std::fixed
        << std::setprecision(6);

    json << "{";

    /*
    ============================================================
    DATA
    ============================================================
    */

    json << "\"data\":[{";
    json << "\"type\":\"surface\",";

    json << "\"z\":";

    writeMatrix(
        json,
        data.frames[0]);

    json << "}],";

    /*
    ============================================================
    LAYOUT
    ============================================================
    */

    json << "\"layout\":{";

    json << "\"title\":\""
        << data.title
        << "\",";

    /*
    ------------------------------------------------------------
    AXES
    ------------------------------------------------------------
    */

    json << "\"scene\":{";

    json << "\"xaxis\":{";
    json << "\"title\":\""
        << data.xLabel
        << "\"";
    json << "},";

    json << "\"yaxis\":{";
    json << "\"title\":\""
        << data.yLabel
        << "\"";
    json << "},";

    json << "\"zaxis\":{";
    json << "\"title\":\""
        << data.zLabel
        << "\"";
    json << "}";

    json << "},";

    /*
    ------------------------------------------------------------
    PLAY / STOP BUTTONS
    ------------------------------------------------------------
    */

    json << "\"updatemenus\":[{";

    json << "\"type\":\"buttons\",";
    json << "\"direction\":\"left\",";
    json << "\"showactive\":false,";

    json << "\"buttons\":[";

    /*
    PLAY
    */

    json << "{";
    json << "\"label\":\"\\u25B6\",";
    json << "\"method\":\"animate\",";
    json << "\"args\":[null,{";
    json << "\"mode\":\"immediate\",";
    json << "\"fromcurrent\":true,";
    json << "\"transition\":{\"duration\":0},";
    json << "\"frame\":{";
	json << "\"duration\":50,"; // sets duration for each frame in milliseconds, 50 meanos 20 frames per second
    json << "\"redraw\":true";
    json << "}";
    json << "}]";
    json << "}";

    /*
    STOP
    */

    json << ",{";
    json << "\"label\":\"\\u25A0\",";
    json << "\"method\":\"animate\",";
    json << "\"args\":[[null],{";
    json << "\"mode\":\"immediate\",";
    json << "\"transition\":{\"duration\":0},";
    json << "\"frame\":{";
    json << "\"duration\":0,";
    json << "\"redraw\":false";
    json << "}";
    json << "}]";
    json << "}";

    json << "]";

    json << "}],";

    /*
    ------------------------------------------------------------
    SLIDER
    ------------------------------------------------------------
    */

    json << "\"sliders\":[{";

    json << "\"active\":0,";
    json << "\"currentvalue\":{";
    json << "\"prefix\":\"Frame: \"";
    json << "},";

    json << "\"steps\":[";

    for (size_t i = 0;
        i < data.frames.size();
        i++)
    {
        if (i > 0)
        {
            json << ",";
        }

        json << "{";

        json << "\"label\":\""
            << i
            << "\",";

        json << "\"method\":\"animate\",";

        json << "\"args\":[[\"f"
            << i
            << "\"],{";

        json << "\"mode\":\"immediate\",";
        json << "\"transition\":{\"duration\":0},";
        json << "\"frame\":{";
        json << "\"duration\":0,";
        json << "\"redraw\":true";
        json << "}";

        json << "}]";

        json << "}";
    }

    json << "]";

    json << "}]";

    json << "},";

    /*
    ============================================================
    FRAMES
    ============================================================
    */

    json << "\"frames\":[";

    for (size_t i = 0;
        i < data.frames.size();
        i++)
    {
        if (i > 0)
        {
            json << ",";
        }

        json << "{";

        json << "\"name\":\"f"
            << i
            << "\",";

        json << "\"traces\":[0],";

        json << "\"data\":[{";

        json << "\"z\":";

        writeMatrix(
            json,
            data.frames[i]);

        json << "}]";

        json << "}";
    }

    json << "],";

    /*
    ============================================================
    CONFIG
    ============================================================
    */

    json << "\"config\":{";
    json << "\"responsive\":true";
    json << "}";

    json << "}";

    return json.str();
}

void JsonBuilder::writeMatrix(
    std::ostringstream& json,
    const std::vector<std::vector<double>>& matrix)
{
    json << "[";

    for (size_t r = 0; r < matrix.size(); r++)
    {
        if (r > 0)
        {
            json << ",";
        }

        json << "[";

        for (size_t c = 0; c < matrix[r].size(); c++)
        {
            if (c > 0)
            {
                json << ",";
            }

            json << matrix[r][c];
        }

        json << "]";
    }

    json << "]";
}