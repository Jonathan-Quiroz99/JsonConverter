#include "animation_frames_writer.h"

void AnimationFramesWriter::writeMatrix(
    std::ostringstream& json,
    const std::vector<std::vector<double>>& matrix)
{
    json << "[";

    for (size_t r = 0; r < matrix.size(); r++)
    {
        if (r)
        {
            json << ",";
        }

        json << "[";

        for (size_t c = 0; c < matrix[r].size(); c++)
        {
            if (c)
            {
                json << ",";
            }

            json << matrix[r][c];
        }

        json << "]";
    }

    json << "]";
}

void AnimationFramesWriter::writeSurfaceFrames(
    std::ostringstream& json,
    const PlotData& data)
{
    json << "\"frames\":[";

    for (size_t i = 0; i < data.frames.size(); i++)
    {
        if (i > 0)
        {
            json << ",";
        }

        json << "{";

        json << "\"name\":\"f" << i << "\",";
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
}

void AnimationFramesWriter::writeScatter3DFrames(
    std::ostringstream& json,
    const PlotData& data)
{
    json << "\"frames\":[";

    for (size_t frameIndex = 0; frameIndex < data.frames.size(); frameIndex++)
    {
        if (frameIndex > 0)
        {
            json << ",";
        }

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

        json << "}]";
        json << "}";
    }

    json << "]";
}