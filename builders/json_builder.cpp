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

std::string JsonBuilder::buildScatter3D(
    const PlotData& data)
{
    if (!data.frames.empty())
    {
        return buildAnimatedScatter3D(data);
    }

    return buildStaticScatter3D(data);
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

std::string JsonBuilder::buildStaticScatter3D(
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

    /*
    ============================================================
    COMPUTE AXIS RANGES ACROSS ALL FRAMES
    ============================================================
    */

    // X and Y use implicit 0-based indices from matrix dimensions.
    // The range is therefore fixed by frame structure, not measured values.

    const double xRawMin = 0.0;
    const double xRawMax = static_cast<double>(data.frames[0][0].size()) - 1.0;
    const double yRawMin = 0.0;
    const double yRawMax = static_cast<double>(data.frames[0].size()) - 1.0;

    const double xPad = (xRawMax - xRawMin) * 0.05;
    const double yPad = (yRawMax - yRawMin) * 0.05;

    const double xMin = xRawMin - xPad;
    const double xMax = xRawMax + xPad;
    const double yMin = yRawMin - yPad;
    const double yMax = yRawMax + yPad;

    // Z: scan every value across all frames for global min/max.

    double zRawMin = data.frames[0][0][0];
    double zRawMax = data.frames[0][0][0];

    for (const auto& frame : data.frames)
    {
        for (const auto& row : frame)
        {
            for (double val : row)
            {
                if (val < zRawMin) zRawMin = val;
                if (val > zRawMax) zRawMax = val;
            }
        }
    }

    const double zPad = (zRawMax - zRawMin) * 0.05;
    const double zMin = zRawMin - zPad;
    const double zMax = zRawMax + zPad;

    json << "{";

    /*
    ============================================================
    DATA
    ============================================================
    */

    json << "\"data\":[{";
    json << "\"type\":\"surface\",";
    json << "\"colorscale\":\"Jet\",";
    json << "\"cmin\":" << zRawMin << ",";
    json << "\"cmax\":" << zRawMax << ",";
    json << "\"showscale\":true,";

    json << "\"colorbar\":{";
    json << "\"x\":1.02,";
    json << "\"title\":{\"text\":\"" << data.zLabel << "\"},";
    json << "\"nticks\":10";
    json << "},";

    json << "\"hovertemplate\":\""
        << data.xLabel << ": %{x}<br>"
        << data.yLabel << ": %{y}<br>"
        << data.zLabel << ": %{z}"
        << "<extra></extra>\",";

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

    json << "\"title\":{";
    json << "\"text\":\""
        << data.title
        << "\"";
    json << "},";

    /*
    ------------------------------------------------------------
    SCENE
    ------------------------------------------------------------
    */

    json << "\"scene\":{";

    // CAMERA

    json << "\"camera\":{";
    json << "\"eye\":{\"x\":1.0,\"y\":1.0,\"z\":1.0}";
    json << "},";

    // X AXIS

    json << "\"xaxis\":{";
    json << "\"title\":{\"text\":\"" << data.xLabel << "\"},";
    json << "\"range\":[" << xMin << "," << xMax << "],";
    json << "\"autorange\":false";
    json << "},";

    // Y AXIS

    json << "\"yaxis\":{";
    json << "\"title\":{\"text\":\"" << data.yLabel << "\"},";
    json << "\"range\":[" << yMin << "," << yMax << "],";
    json << "\"autorange\":false";
    json << "},";

    // Z AXIS

    json << "\"zaxis\":{";
    json << "\"title\":{\"text\":\"" << data.zLabel << "\"},";
    json << "\"range\":[" << zMin << "," << zMax << "],";
    json << "\"autorange\":false";
    json << "},";

    // ASPECT RATIO

    json << "\"aspectmode\":\"manual\",";
    json << "\"aspectratio\":{\"x\":1,\"y\":1,\"z\":1}";

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
    json << "\"x\":0.0,";
    json << "\"y\":0.0,";
    json << "\"yanchor\":\"top\",";
    json << "\"pad\":{\"t\":35,\"r\":10},";

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
    json << "\"pad\":{\"t\":5,\"b\":10},";
    json << "\"x\":0.05,";
    json << "\"len\":0.9,";

    json << "\"currentvalue\":{";
    json << "\"prefix\":\"Crank Angle: \"";
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

    json << "]";

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

std::string JsonBuilder::buildAnimatedScatter3D(
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
    INITIAL TRACE
    ============================================================
    */

    json << "\"data\":[{";

    json << "\"type\":\"scatter3d\",";
    json << "\"mode\":\"lines+markers\",";

    /*
    FRAME 0
    */

    const auto& firstFrame =
        data.frames[0];

    json << "\"x\":[";
    for (size_t i = 0; i < firstFrame.size(); i++)
    {
        if (i > 0) json << ",";
        json << firstFrame[i][0];
    }
    json << "],";

    json << "\"y\":[";
    for (size_t i = 0; i < firstFrame.size(); i++)
    {
        if (i > 0) json << ",";
        json << firstFrame[i][1];
    }
    json << "],";

    json << "\"z\":[";
    for (size_t i = 0; i < firstFrame.size(); i++)
    {
        if (i > 0) json << ",";
        json << firstFrame[i][2];
    }
    json << "]";

    json << "}],";

    /*
    ============================================================
    LAYOUT
    ============================================================
    */

    json << "\"layout\":{";

    json << "\"title\":{";
    json << "\"text\":\""
        << data.title
        << "\"";
    json << "},";

    json << "\"scene\":{";

    json << "\"xaxis\":{";
    json << "\"title\":{";
    json << "\"text\":\""
        << data.xLabel
        << "\"";
    json << "}";
    json << "},";

    json << "\"yaxis\":{";
    json << "\"title\":{";
    json << "\"text\":\""
        << data.yLabel
        << "\"";
    json << "}";
    json << "},";

    json << "\"zaxis\":{";
    json << "\"title\":{";
    json << "\"text\":\""
        << data.zLabel
        << "\"";
    json << "}";
    json << "}";

    json << "},";

    /*
    PLAY / STOP
    */

    json << "\"updatemenus\":[{";
    json << "\"type\":\"buttons\",";
    json << "\"showactive\":false,";

    json << "\"buttons\":[";

    json << "{";
    json << "\"label\":\"Play\",";
    json << "\"method\":\"animate\",";
    json << "\"args\":[null,{";
    json << "\"mode\":\"immediate\",";
    json << "\"fromcurrent\":true,";
    json << "\"transition\":{\"duration\":0},";
    json << "\"frame\":{\"duration\":50,\"redraw\":true}";
    json << "}]";
    json << "}";

    json << ",{";
    json << "\"label\":\"Stop\",";
    json << "\"method\":\"animate\",";
    json << "\"args\":[[null],{";
    json << "\"mode\":\"immediate\",";
    json << "\"transition\":{\"duration\":0},";
    json << "\"frame\":{\"duration\":0,\"redraw\":false}";
    json << "}]";
    json << "}";

    json << "]";
    json << "}],";

    /*
    ============================================================
    SLIDER
    ============================================================
    */

    json << "\"sliders\":[{";
    json << "\"active\":0,";
    json << "\"currentvalue\":{\"prefix\":\"Frame: \"},";
    json << "\"steps\":[";

    for (size_t i = 0; i < data.frames.size(); i++)
    {
        if (i > 0)
        {
            json << ",";
        }

        json << "{";
        json << "\"label\":\"" << i << "\",";
        json << "\"method\":\"animate\",";
        json << "\"args\":[[\"f" << i << "\"],{";
        json << "\"mode\":\"immediate\",";
        json << "\"transition\":{\"duration\":0},";
        json << "\"frame\":{\"duration\":0,\"redraw\":true}";
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

    for (size_t frameIndex = 0;
        frameIndex < data.frames.size();
        frameIndex++)
    {
        if (frameIndex > 0)
        {
            json << ",";
        }

        const auto& frame =
            data.frames[frameIndex];

        json << "{";

        json << "\"name\":\"f"
            << frameIndex
            << "\",";

        json << "\"traces\":[0],";

        json << "\"data\":[{";

        json << "\"x\":[";
        for (size_t i = 0; i < frame.size(); i++)
        {
            if (i > 0) json << ",";
            json << frame[i][0];
        }
        json << "],";

        json << "\"y\":[";
        for (size_t i = 0; i < frame.size(); i++)
        {
            if (i > 0) json << ",";
            json << frame[i][1];
        }
        json << "],";

        json << "\"z\":[";
        for (size_t i = 0; i < frame.size(); i++)
        {
            if (i > 0) json << ",";
            json << frame[i][2];
        }
        json << "]";

        json << "}]";

        json << "}";
    }

    json << "]";

    json << "}";

    return json.str();
}