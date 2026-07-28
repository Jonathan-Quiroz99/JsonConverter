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
    json << "\"" << axisName << "\":{";

    json << "\"title\":{";

    json << "\"text\":\""
        << title
        << "\"";

    json << "}";

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