#include "layout_writer.h"

void LayoutWriter::writeTitle(
    std::ostringstream& json,
    const std::string& title)
{
    json << "\"title\":{";
    json << "\"text\":\""
        << title
        << "\"";
    json << "}";
}

void LayoutWriter::writeAxis(
    std::ostringstream& json,
    const std::string& axisName,
    const std::string& title)
{
    json << "\""
         << axisName
         << "\":{";

    json << "\"title\":{";
    json << "\"text\":\""
         << title
         << "\"";
    json << "}";

    json << "}";
}

void LayoutWriter::writeAnimatedAxis(
    std::ostringstream& json,
    const std::string& axisName,
    const std::string& title,
    double min,
    double max)
{
    json << "\""
        << axisName
        << "\":{";

    json << "\"title\":{";
    json << "\"text\":\""
        << title
        << "\"";
    json << "},";

    json << "\"range\":["
        << min
        << ","
        << max
        << "],";

    json << "\"autorange\":false";

    json << "}";
}

void LayoutWriter::writeCamera(
    std::ostringstream& json)
{
    json << "\"camera\":{";

    json << "\"eye\":{";

    json << "\"x\":1.0,";
    json << "\"y\":1.0,";
    json << "\"z\":1.0";

    json << "}";

    json << "}";
}

void LayoutWriter::writeAspectRatio(
    std::ostringstream& json)
{
    json << "\"aspectmode\":\"manual\",";
    json << "\"aspectratio\":{";
    json << "\"x\":1,";
    json << "\"y\":1,";
    json << "\"z\":1";
    json << "}";
}

void LayoutWriter::writeMargin(
    std::ostringstream& json,
    int top,
    int bottom,
    int left,
    int right)
{
    json << "\"margin\":{";

    json << "\"t\":"
        << top
        << ",";

    json << "\"b\":"
        << bottom
        << ",";

    json << "\"l\":"
        << left
        << ",";

    json << "\"r\":"
        << right;

    json << "}";
}

void LayoutWriter::writeScene(
    std::ostringstream& json,
    const PlotData& data)
{
    json << "\"scene\":{";

    writeAxis(
        json,
        "xaxis",
        data.xLabel);

    json << ",";

    writeAxis(
        json,
        "yaxis",
        data.yLabel);

    json << ",";

    writeAxis(
        json,
        "zaxis",
        data.zLabel);

    json << "}";

}

void LayoutWriter::writeAnimatedScene(
    std::ostringstream& json,
    const PlotData& data,
    double xMin,
    double xMax,
    double yMin,
    double yMax,
    double zMin,
    double zMax)
{
    json << "\"scene\":{";

    // CAMERA

    writeCamera(
        json);

    json << ",";

    // X AXIS

    writeAnimatedAxis(
        json,
        "xaxis",
        data.xLabel,
        xMin,
        xMax);

    json << ",";

    // Y AXIS

    writeAnimatedAxis(
        json,
        "yaxis",
        data.yLabel,
        yMin,
        yMax);

    json << ",";

    // Z AXIS

    writeAnimatedAxis(
        json,
        "zaxis",
        data.zLabel,
        zMin,
        zMax);

    json << ",";

    // ASPECT RATIO

    writeAspectRatio(
        json);

    json << "}";

}

void LayoutWriter::write2DLayout(
    std::ostringstream& json,
    const PlotData& data)
{
    writeTitle(
        json,
        data.title);

    json << ",";

    writeAxis(
        json,
        "xaxis",
        data.xLabel);

    json << ",";

    writeAxis(
        json,
        "yaxis",
        data.yLabel);
}

void LayoutWriter::write3DLayout(
    std::ostringstream& json,
    const PlotData& data)
{
    writeTitle(
        json,
        data.title);

    json << ",";

    writeScene(
        json,
        data);
}