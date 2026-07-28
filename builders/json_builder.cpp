#include "json_builder.h"
#include "layout/layout_writer.h"
#include "animation/animation_controls_writer.h"

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

        json << "],";

        json << "\"line\":{\"width\":2},";
        json << "\"marker\":{\"size\":2},";
        json << "\"hovertemplate\":\""
            << data.xLabel << ": %{x}<br>"
            << series.name << ": %{y}"
            << "<extra></extra>\"";

        json << "}";
    }

    json << "],";

    json << "\"layout\":{";

    LayoutWriter::write2DLayout(
        json,
        data);

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

    json << "\"z\":";

    writeMatrix(
        json,
        data.matrix);

    json << "}";

    json << "],";

    json << "\"layout\":{";

    LayoutWriter::write3DLayout(
        json,
        data);

    json << "}";

    json << "}";

    return json.str();
}

std::string JsonBuilder::buildHeatmap(
    const PlotData& data)
{
    std::ostringstream json;

    json << std::fixed
        << std::setprecision(6);

    json << "{";

    json << "\"data\":[";

    json << "{";

    json << "\"type\":\"heatmap\",";
    json << "\"colorscale\":\"Viridis\",";

    json << "\"z\":";

    writeMatrix(
        json,
        data.matrix);

    json << "}";

    json << "],";

    json << "\"layout\":{";

    LayoutWriter::write2DLayout(
        json,
        data);

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

    json << "],";

    json << "\"layout\":{";

    LayoutWriter::write3DLayout(
        json,
        data);

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

    LayoutWriter::writeTitle(
        json,
        data.title);

    json << ",";

    /*
    ------------------------------------------------------------
    SCENE
    ------------------------------------------------------------
    */

    LayoutWriter::writeAnimatedScene(
        json,
        data,
        xMin,
        xMax,
        yMin,
        yMax,
        zMin,
        zMax);

    json << ",";

    /*
    ------------------------------------------------------------
    PLAY / STOP BUTTONS
    ------------------------------------------------------------
    */

    AnimationControlsWriter::writeButtons(
        json);

    json << ",";

    /*
    ------------------------------------------------------------
    SLIDER
    ------------------------------------------------------------
    */

    AnimationControlsWriter::writeSlider(
        json,
        data,
        5);

    json << "}";

    json << ",";

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

    /*
    ============================================================
    COMPUTE AXIS RANGES AND TWIST RANGE ACROSS ALL FRAMES
    ============================================================
    */

    double xRawMin = data.frames[0][0][0];
    double xRawMax = data.frames[0][0][0];
    double yRawMin = data.frames[0][0][1];
    double yRawMax = data.frames[0][0][1];
    double zRawMin = data.frames[0][0][2];
    double zRawMax = data.frames[0][0][2];
    double twistMin = data.frames[0][0][3];
    double twistMax = data.frames[0][0][3];

    for (const auto& frame : data.frames)
    {
        for (const auto& node : frame)
        {
            if (node[0] < xRawMin) xRawMin = node[0];
            if (node[0] > xRawMax) xRawMax = node[0];
            if (node[1] < yRawMin) yRawMin = node[1];
            if (node[1] > yRawMax) yRawMax = node[1];
            if (node[2] < zRawMin) zRawMin = node[2];
            if (node[2] > zRawMax) zRawMax = node[2];
            if (node[3] < twistMin) twistMin = node[3];
            if (node[3] > twistMax) twistMax = node[3];
        }
    }

    const double xPad = (xRawMax - xRawMin) * 0.05;
    const double yPad = (yRawMax - yRawMin) * 0.05;
    const double zPad = (zRawMax - zRawMin) * 0.05;

    const double xMin = xRawMin - xPad;
    const double xMax = xRawMax + xPad;
    const double yMin = yRawMin - yPad;
    const double yMax = yRawMax + yPad;
    const double zMin = zRawMin - zPad;
    const double zMax = zRawMax + zPad;

    json << "{";

    /*
    ============================================================
    DATA
    ============================================================
    */

    const auto& firstFrame = data.frames[0];

    json << "\"data\":[{";
    json << "\"type\":\"scatter3d\",";
    json << "\"mode\":\"lines+markers\",";
    json << "\"name\":\"Points\",";

    // X

    json << "\"x\":[";
    for (size_t i = 0; i < firstFrame.size(); i++)
    {
        if (i > 0) json << ",";
        json << firstFrame[i][0];
    }
    json << "],";

    // Y

    json << "\"y\":[";
    for (size_t i = 0; i < firstFrame.size(); i++)
    {
        if (i > 0) json << ",";
        json << firstFrame[i][1];
    }
    json << "],";

    // Z

    json << "\"z\":[";
    for (size_t i = 0; i < firstFrame.size(); i++)
    {
        if (i > 0) json << ",";
        json << firstFrame[i][2];
    }
    json << "],";

    // CUSTOM DATA (TWIST)

    json << "\"customdata\":[";
    for (size_t i = 0; i < firstFrame.size(); i++)
    {
        if (i > 0) json << ",";
        json << firstFrame[i][3];
    }
    json << "],";

    // HOVER TEMPLATE

    json << "\"hovertemplate\":\"";
    json << "Node #: %{pointNumber}<br>";
    json << data.xLabel << ": %{x}<br>";
    json << data.yLabel << ": %{y}<br>";
    json << data.zLabel << ": %{z}<br>";
    json << "Twist: %{customdata}\\u00b0";
    json << "<extra></extra>\",";

    // LINE

    json << "\"line\":{";
    json << "\"color\":\"#378ADD\",";
    json << "\"width\":4";
    json << "},";

    // MARKER

    json << "\"marker\":{";
    json << "\"size\":7,";

    json << "\"color\":[";
    for (size_t i = 0; i < firstFrame.size(); i++)
    {
        if (i > 0) json << ",";
        json << firstFrame[i][3];
    }
    json << "],";

    json << "\"colorscale\":\"Jet\",";
    json << "\"cmin\":" << twistMin << ",";
    json << "\"cmax\":" << twistMax << ",";
    json << "\"showscale\":true,";

    json << "\"colorbar\":{";
    json << "\"x\":1.02,";
    json << "\"title\":{\"text\":\"Twist (\\u00b0)\"},";
    json << "\"nticks\":10";
    json << "},";

    json << "\"opacity\":0.95,";

    json << "\"line\":{";
    json << "\"color\":\"#ffffff\",";
    json << "\"width\":0.5";
    json << "}";

    json << "}";    // closes marker

    json << "}],";  // closes trace and data array

    /*
    ============================================================
    LAYOUT
    ============================================================
    */

    json << "\"layout\":{";

    json << "\"title\":{";
    json << "\"text\":\"" << data.title << "\"";
    json << "},";

    /*
    ------------------------------------------------------------
    SCENE
    ------------------------------------------------------------
    */

    json << "\"scene\":{";

    // ASPECT RATIO

    json << "\"aspectmode\":\"manual\",";
    json << "\"aspectratio\":{\"x\":1,\"y\":1,\"z\":1},";

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

    // CAMERA

    json << "\"camera\":{";
    json << "\"eye\":{\"x\":1.8,\"y\":1.8,\"z\":0.7}";
    json << "}";

    json << "},";   // closes scene

    /*
    ------------------------------------------------------------
    MARGIN
    ------------------------------------------------------------
    */

    json << "\"margin\":{\"t\":50,\"b\":60,\"l\":0,\"r\":0},";

    /*
    PLAY / STOP
    */

    json << "\"updatemenus\":[{";
    json << "\"type\":\"buttons\",";
    json << "\"direction\":\"left\",";
    json << "\"showactive\":false,";
    json << "\"x\":0.0,";
    json << "\"y\":-0.05,";
    json << "\"xanchor\":\"left\",";
    json << "\"yanchor\":\"top\",";
    json << "\"pad\":{\"t\":10,\"r\":10},";

    json << "\"buttons\":[";

    // PLAY

    json << "{";
    json << "\"label\":\"\\u25B6\",";
    json << "\"method\":\"animate\",";
    json << "\"args\":[null,{";
    json << "\"mode\":\"immediate\",";
    json << "\"fromcurrent\":true,";
    json << "\"transition\":{\"duration\":0},";
    json << "\"frame\":{";
    json << "\"duration\":50,";
    json << "\"redraw\":true";
    json << "}";
    json << "}]";
    json << "}";

    // STOP

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
    json << "}],";  // closes updatemenus

    /*
    ------------------------------------------------------------
    SLIDER
    ------------------------------------------------------------
    */

    json << "\"sliders\":[{";
    json << "\"active\":0,";
    json << "\"pad\":{\"t\":40,\"b\":10},";
    json << "\"x\":0.05,";
    json << "\"len\":0.9,";

    json << "\"currentvalue\":{";
    json << "\"prefix\":\"Crank Angle: \",";
    json << "\"visible\":true,";
    json << "\"xanchor\":\"center\",";
    json << "\"font\":{\"size\":12}";
    json << "},";

    json << "\"steps\":[";

    for (size_t i = 0; i < data.frames.size(); i++)
    {
        if (i > 0) json << ",";

        json << "{";
        json << "\"label\":\"" << i << "\",";
        json << "\"method\":\"animate\",";
        json << "\"args\":[[\"f" << i << "\"],{";
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
    json << "}]";   // closes sliders

    json << "},";   // closes layout

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
        if (frameIndex > 0) json << ",";

        const auto& frame = data.frames[frameIndex];

        json << "{";
        json << "\"name\":\"f" << frameIndex << "\",";
        json << "\"traces\":[0],";
        json << "\"data\":[{";

        // X

        json << "\"x\":[";
        for (size_t i = 0; i < frame.size(); i++)
        {
            if (i > 0) json << ",";
            json << frame[i][0];
        }
        json << "],";

        // Y

        json << "\"y\":[";
        for (size_t i = 0; i < frame.size(); i++)
        {
            if (i > 0) json << ",";
            json << frame[i][1];
        }
        json << "],";

        // Z

        json << "\"z\":[";
        for (size_t i = 0; i < frame.size(); i++)
        {
            if (i > 0) json << ",";
            json << frame[i][2];
        }
        json << "],";

        // CUSTOM DATA (TWIST)

        json << "\"customdata\":[";
        for (size_t i = 0; i < frame.size(); i++)
        {
            if (i > 0) json << ",";
            json << frame[i][3];
        }
        json << "],";

        // MARKER COLOR (TWIST)

        json << "\"marker\":{\"color\":[";
        for (size_t i = 0; i < frame.size(); i++)
        {
            if (i > 0) json << ",";
            json << frame[i][3];
        }
        json << "]}";

        json << "}]";   // closes data trace and data array
        json << "}";    // closes frame object
    }

    json << "]";    // closes frames array
    json << "}";    // closes root

    return json.str();
}