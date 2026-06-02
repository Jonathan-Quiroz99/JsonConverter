#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>
#include <map>

#include "../classifiers/classification.h"

/*
============================================================
FILE TYPES
============================================================
*/

enum class FileType
{
    UNKNOWN = 0,

    /*
    2D
    */

    TIME_SERIES_2D = 1,
    CORRELATION_2D = 2,
    GRID_2D = 3,

    /*
    3D
    */

    MESH_3D_SURFACE = 4,
    MESH_3D_VOLUME = 5,

    /*
    SPECIALIZED
    */

    RING_WEAR = 6
};

/*
============================================================
PARSED DATA
============================================================
*/

struct ParsedData
{
    /*
    ========================================================
    BASIC INFO
    ========================================================
    */

    FileType type = FileType::UNKNOWN;

    PlotType detected_plot = PlotType::UNKNOWN;

    DataType detected_data = DataType::UNKNOWN;

    std::string data_type;

    // Added plot_type to match JSON builder expectations
    std::string plot_type;

    std::string filename;

    /*
    ========================================================
    RAW NUMERIC DATA
    ========================================================
    */

    std::vector<std::vector<double>> numeric_data;

    /*
    ========================================================
    HEADERS / LABELS
    ========================================================
    */

    std::vector<std::string> headers;

    std::vector<std::string> units;

    /*
    ========================================================
    SECTIONED DATA
    Example:
        STRESS
            component 1
            component 2
    ========================================================
    */

    struct SectionData
    {
        std::map<
            int,
            std::vector<std::vector<double>>
        > components;
    };

    std::map<std::string, SectionData> sections;

    /*
    ========================================================
    MESH DATA
    FOR PLOTLY MESH3D
    ========================================================
    */

    struct MeshData
    {
        /*
        VERTEX POSITIONS
        */

        std::vector<double> x;
        std::vector<double> y;
        std::vector<double> z;

        /*
        TRIANGLE INDICES
        */

        std::vector<int> i;
        std::vector<int> j;
        std::vector<int> k;

        /*
        OPTIONAL COLOR/SCALAR DATA
        */

        std::vector<double> intensity;

        /*
        OPTIONAL EXTRA
        */

        std::vector<int> node_ids;

        std::vector<int> element_ids;
    };

    MeshData mesh;

    /*
    ========================================================
    METADATA
    ========================================================
    */

    std::map<std::string, std::string> metadata;

    /*
    ========================================================
    DATA SHAPE
    ========================================================
    */

    struct DataShape
    {
        size_t rows = 0;

        size_t cols = 0;

        bool is_matrix = false;

        bool is_sparse = false;

        bool is_surface = false;

        bool is_mesh = false;

        std::string layout = "unknown";
    };

    DataShape shape;

    /*
    ========================================================
    HELPER FLAGS
    ========================================================
    */

    bool has_2d_data() const
    {
        return !numeric_data.empty();
    }

    bool has_sections() const
    {
        return !sections.empty();
    }

    bool has_mesh() const
    {
        return !mesh.x.empty();
    }

    bool is_matrix_data() const
    {
        return
            !numeric_data.empty() &&
            shape.is_matrix &&
            numeric_data.size() > 1 &&
            numeric_data[0].size() > 1;
    }

    bool is_surface_data() const
    {
        return shape.is_surface;
    }

    bool is_mesh_data() const
    {
        return shape.is_mesh;
    }
};

#endif // CONFIG_H