#include "animation_controls_writer.h"

void AnimationControlsWriter::writeButtons(
    std::ostringstream& json,
    int playDuration)
{
    json << "\"updatemenus\":[{";

    json << "\"type\":\"buttons\",";
    json << "\"direction\":\"left\",";
    json << "\"showactive\":false,";
    json << "\"x\":0.0,";
    json << "\"y\":0.0,";
    json << "\"yanchor\":\"top\",";
    json << "\"pad\":{\"t\":35,\"r\":10},";

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
    json << "\"duration\":" << playDuration << ",";
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

    json << "}]";
}

void AnimationControlsWriter::writeSlider(
    std::ostringstream& json,
    const PlotData& data,
    int topPadding,
    const std::string& prefix)
{
    json << "\"sliders\":[{";

    json << "\"active\":0,";
    json << "\"pad\":{\"t\":" << topPadding << ",\"b\":10},";
    json << "\"x\":0.05,";
    json << "\"len\":0.9,";

    json << "\"currentvalue\":{";
    json << "\"prefix\":\"" << prefix << "\"";
    json << "},";

    json << "\"steps\":[";

    for (size_t i = 0; i < data.frames.size(); i++)
    {
        if (i > 0)
            json << ",";

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
}