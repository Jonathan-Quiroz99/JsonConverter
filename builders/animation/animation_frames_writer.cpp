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