#ifndef PARSER_GMSH_H
#define PARSER_GMSH_H

#include "base_parser.h"
#include <set>
#include <tuple>
#include <sstream>      
#include <cctype>       


class ParserGmsh : public BaseParser {
public:
    ParserGmsh(const std::string& path, std::function<void(const std::string&)> callback = nullptr)
        : BaseParser(path, callback) {
    }

    ParsedData parse() override {
        log("  Parsing as Gmsh file");

        ParsedData result;
        result.filename = filename;
        result.type = FileType::MESH_3D_VOLUME;
        result.data_type = "gmsh_mesh";

        std::map<int, std::tuple<double, double, double>> nodes;
        std::vector<std::tuple<std::string, std::vector<int>>> elements;
        std::map<int, double> node_data;

        std::ifstream file(filepath);
        if (!file.is_open()) {
            log("  Error opening file");
            return result;
        }

        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        file.close();

        std::string state = "HEADER";
        size_t i = 0;

        while (i < lines.size()) {
            line = lines[i];
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            if (line == "$MeshFormat") {
                state = "FORMAT";
                i += 2;
                continue;
            }
            else if (line == "$Nodes") {
                state = "NODES";
                i++;
                if (i < lines.size()) {
                    try {
                        int node_count = std::stoi(lines[i]);
                        i++;

                        for (int n = 0; n < node_count && i < lines.size(); n++, i++) {
                            std::istringstream iss(lines[i]);  // Ahora funciona
                            int id;
                            double x, y, z;
                            if (iss >> id >> x >> y >> z) {
                                nodes[id] = std::make_tuple(x, y, z);
                            }
                        }
                    }
                    catch (...) {
                        i++;
                    }
                }
                continue;
            }
            else if (line == "$Elements") {
                state = "ELEMENTS";
                i++;
                if (i < lines.size()) {
                    try {
                        int elem_count = std::stoi(lines[i]);
                        i++;

                        for (int e = 0; e < elem_count && i < lines.size(); e++, i++) {
                            std::istringstream iss(lines[i]);
                            int id, type;
                            iss >> id >> type;

                            // Only triangles (type 2)
                            if (type == 2) {
                                std::vector<int> all_nums;
                                int num;
                                while (iss >> num) {
                                    all_nums.push_back(num);
                                }
                                if (all_nums.size() >= 3) {
                                    int n1 = all_nums[all_nums.size() - 3];
                                    int n2 = all_nums[all_nums.size() - 2];
                                    int n3 = all_nums[all_nums.size() - 1];
                                    elements.push_back(std::make_tuple("triangle",
                                        std::vector<int>{n1, n2, n3}));
                                }
                            }
                        }
                    }
                    catch (...) {
                        i++;
                    }
                }
                continue;
            }
            else if (line == "$NodeData") {
                state = "NODEDATA";
                i += 5;  // Skip headers

                if (i < lines.size()) {
                    try {
                        int data_count = std::stoi(lines[i]);
                        i++;

                        for (int d = 0; d < data_count && i < lines.size(); d++, i++) {
                            std::istringstream iss(lines[i]);
                            int node_id;
                            double val;
                            std::string val_str;
                            if (iss >> node_id >> val_str) {
                                // Replace D/d with e
                                for (char& c : val_str) {
                                    if (c == 'D' || c == 'd') c = 'e';
                                }
                                val = std::stod(val_str);
                                node_data[node_id] = val;
                            }
                        }
                    }
                    catch (...) {
                        i++;
                    }
                }
                continue;
            }
            else {
                i++;
            }
        }

        log("  Nodes: " + std::to_string(nodes.size()));
        log("  Elements: " + std::to_string(elements.size()));
        log("  Nodal data: " + std::to_string(node_data.size()));

        // Extract surface faces
        std::vector<std::vector<int>> surface_faces;
        for (const auto& elem : elements) {
            if (std::get<0>(elem) == "triangle") {
                surface_faces.push_back(std::get<1>(elem));
            }
        }

        // Build mesh
        result.mesh = build_mesh(nodes, surface_faces, node_data);

        if (!result.mesh.x.empty()) {
            log("  Mesh vertices: " + std::to_string(result.mesh.x.size()));
            log("  Mesh triangles: " + std::to_string(result.mesh.i.size()));
        }

        result.metadata["nodes"] = std::to_string(nodes.size());
        result.metadata["elements"] = std::to_string(elements.size());

        return result;
    }

private:
    ParsedData::MeshData build_mesh(const std::map<int, std::tuple<double, double, double>>& nodes,
        const std::vector<std::vector<int>>& faces,
        const std::map<int, double>& node_data) {
        ParsedData::MeshData mesh;

        if (faces.empty() || nodes.empty()) {
            return mesh;
        }

        // Collect unique nodes used in faces
        std::set<int> used_nodes;
        for (const auto& face : faces) {
            for (int nid : face) {
                used_nodes.insert(nid);
            }
        }

        // Map IDs to consecutive indices
        std::vector<int> node_list(used_nodes.begin(), used_nodes.end());
        std::map<int, int> node_index;
        for (size_t i = 0; i < node_list.size(); i++) {
            node_index[node_list[i]] = static_cast<int>(i);
        }

        // Extract coordinates
        for (int nid : node_list) {
            auto it = nodes.find(nid);
            if (it != nodes.end()) {
                mesh.x.push_back(std::get<0>(it->second));
                mesh.y.push_back(std::get<1>(it->second));
                mesh.z.push_back(std::get<2>(it->second));
            }
        }

        // Extract triangle indices
        for (const auto& face : faces) {
            if (face.size() == 3) {
                mesh.i.push_back(node_index[face[0]]);
                mesh.j.push_back(node_index[face[1]]);
                mesh.k.push_back(node_index[face[2]]);
            }
        }

        // Extract intensities
        for (int nid : node_list) {
            auto it = node_data.find(nid);
            if (it != node_data.end()) {
                mesh.intensity.push_back(it->second);
            }
            else {
                // Use z-coordinate as fallback
                auto node_it = nodes.find(nid);
                if (node_it != nodes.end()) {
                    mesh.intensity.push_back(std::get<2>(node_it->second));
                }
            }
        }

        return mesh;
    }
};

#endif // PARSER_GMSH_H