#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>
#include <map>

enum class FileType {
    UNKNOWN = 0,
    TIME_SERIES_2D = 1,
    CORRELATION_2D = 2,
    GRID_2D = 3,
    MESH_3D_SURFACE = 4,
    MESH_3D_VOLUME = 5,
    RING_WEAR = 6
};

struct ParsedData {
    FileType type = FileType::UNKNOWN;
    std::string data_type;
    std::string filename;

    // For 2D data
    std::vector<std::vector<double>> numeric_data;
    std::vector<std::string> headers;
    std::vector<std::string> units;

    // For 3D data sections
    struct SectionData {
        std::map<int, std::vector<std::vector<double>>> components;
    };
    std::map<std::string, SectionData> sections;

    // For mesh data
    struct MeshData {
        std::vector<double> x, y, z;
        std::vector<int> i, j, k;
        std::vector<double> intensity;
    };
    MeshData mesh;

    // Metadata (abstract, extensible)
    std::map<std::string, std::string> metadata;

    // NEW: Data shape information
    struct DataShape {
        size_t rows = 0;
        size_t cols = 0;
        bool is_matrix = false;
        bool is_sparse = false;
        std::string layout = "unknown"; // "grid", "ragged", "sparse"
    };
    DataShape shape;

    // Methods
    bool has_2d_data() const { return !numeric_data.empty(); }
    bool has_sections() const { return !sections.empty(); }
    bool has_mesh() const { return !mesh.x.empty(); }

    // NEW: Helper to check if data is a matrix
    bool is_matrix_data() const {
        return !numeric_data.empty() && shape.is_matrix && numeric_data.size() > 1 && numeric_data[0].size() > 1;
    }
};

#endif // CONFIG_H