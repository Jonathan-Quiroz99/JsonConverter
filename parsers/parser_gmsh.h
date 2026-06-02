#ifndef PARSER_GMSH_H
#define PARSER_GMSH_H

#include "base_parser.h"

#include <set>
#include <tuple>
#include <sstream>
#include <cctype>
#include <map>
#include <vector>
#include <string>

class ParserGmsh : public BaseParser
{
  public:
    ParserGmsh(
      const std::string &path,
      std::function<void(const std::string &)> callback = nullptr)
      : BaseParser(path, callback)
    {
    }

    ParsedData parse() override
    {
      log("Parsing GMSH mesh");

      ParsedData result;

      result.filename = filename;
      result.type = FileType::MESH_3D_VOLUME;
      result.data_type = "gmsh_mesh";

      std::ifstream file(filepath);

      if (!file.is_open())
      {
        log("Failed opening file");
        return result;
      }

      std::vector<std::string> lines;
      std::string line;

      while (std::getline(file, line))
      {
        lines.push_back(line);
      }

      file.close();

      /*
      ============================================================
      STORAGE
      ============================================================
      */

      std::map<int, std::tuple<double, double, double>> nodes;

      std::vector<std::vector<int>> triangles;

      std::map<int, double> node_values;

      /*
      ============================================================
      PARSE
      ============================================================
      */

      size_t i = 0;

      while (i < lines.size())
      {
        std::string current = trim(lines[i]);

        /*
        ========================================================
        NODES
        ========================================================
        */

        if (current == "$Nodes")
        {
          i++;

          if (i >= lines.size())
          {
            break;
          }

          int node_count = safe_stoi(trim(lines[i]));

          i++;

          for (int n = 0; n < node_count && i < lines.size(); n++, i++)
          {
            std::istringstream iss(lines[i]);

            int id;
            double x;
            double y;
            double z;

            if (iss >> id >> x >> y >> z)
            {
              nodes[id] =
                {
                  x,
                  y,
                  z
                };
            }
          }

          continue;
        }

        /*
        ========================================================
        ELEMENTS
        ========================================================
        */

        if (current == "$Elements")
        {
          i++;

          if (i >= lines.size())
          {
            break;
          }

          int element_count = safe_stoi(trim(lines[i]));

          i++;

          for (int e = 0; e < element_count && i < lines.size(); e++, i++)
          {
            std::istringstream iss(lines[i]);

            int element_id;
            int element_type;

            iss >> element_id >> element_type;

            /*
            GMSH TRIANGLE TYPE = 2
            */

            if (element_type != 2)
            {
              continue;
            }

            std::vector<int> values;

            int value;

            while (iss >> value)
            {
              values.push_back(value);
            }

            if (values.size() < 3)
            {
              continue;
            }

            /*
            LAST 3 VALUES ARE NODE IDS
            */

            int n1 = values[values.size() - 3];
            int n2 = values[values.size() - 2];
            int n3 = values[values.size() - 1];

            triangles.push_back(
              {
                n1,
                n2,
                n3
              });
          }

          continue;
        }

        /*
        ========================================================
        NODE DATA
        ========================================================
        */

        if (current == "$NodeData")
        {
          /*
          SIMPLE GMSH NODEDATA SUPPORT
          */

          i++;

          while (
            i < lines.size() &&
            trim(lines[i]) != "$EndNodeData")
          {
            std::string row = trim(lines[i]);

            std::istringstream iss(row);

            int node_id;
            std::string value_str;

            if (iss >> node_id >> value_str)
            {
              for (char &c : value_str)
              {
                if (c == 'D' || c == 'd')
                {
                  c = 'e';
                }
              }

              try
              {
                node_values[node_id] =
                  std::stod(value_str);
              }
              catch (...)
              {
              }
            }

            i++;
          }

          continue;
        }

        i++;
      }

      /*
      ============================================================
      BUILD INTERNAL MESH
      ============================================================
      */

      build_mesh(
        result,
        nodes,
        triangles,
        node_values);

      /*
      ============================================================
      METADATA
      ============================================================
      */

      result.metadata["nodes"] =
        std::to_string(nodes.size());

      result.metadata["triangles"] =
        std::to_string(triangles.size());

      log(
        "Nodes: " +
        std::to_string(nodes.size()));

      log(
        "Triangles: " +
        std::to_string(triangles.size()));

      return result;
    }

  private:
    void build_mesh(
      ParsedData &result,
      const std::map<int, std::tuple<double, double, double>> &nodes,
      const std::vector<std::vector<int>> &triangles,
      const std::map<int, double> &node_values)
    {
      /*
      ============================================================
      MAP NODE IDs TO ARRAY INDEX
      ============================================================
      */

      std::vector<int> node_ids;

      for (const auto &pair : nodes)
      {
        node_ids.push_back(pair.first);
      }

      std::map<int, int> index_map;

      for (size_t i = 0; i < node_ids.size(); i++)
      {
        index_map[node_ids[i]] =
          static_cast<int>(i);
      }

      /*
      ============================================================
      VERTICES
      ============================================================
      */

      for (int node_id : node_ids)
      {
        auto it = nodes.find(node_id);

        if (it == nodes.end())
        {
          continue;
        }

        result.mesh.x.push_back(
          std::get<0>(it->second));

        result.mesh.y.push_back(
          std::get<1>(it->second));

        result.mesh.z.push_back(
          std::get<2>(it->second));

        /*
        ========================================================
        INTENSITY
        ========================================================
        */

        auto value_it =
          node_values.find(node_id);

        if (value_it != node_values.end())
        {
          result.mesh.intensity.push_back(
            value_it->second);
        }
        else
        {
          result.mesh.intensity.push_back(
            std::get<2>(it->second));
        }
      }

      /*
      ============================================================
      TRIANGLES
      ============================================================
      */

      for (const auto &triangle : triangles)
      {
        if (triangle.size() != 3)
        {
          continue;
        }

        int n1 = triangle[0];
        int n2 = triangle[1];
        int n3 = triangle[2];

        if (
          index_map.find(n1) == index_map.end() ||
          index_map.find(n2) == index_map.end() ||
          index_map.find(n3) == index_map.end())
        {
          continue;
        }

        result.mesh.i.push_back(index_map[n1]);
        result.mesh.j.push_back(index_map[n2]);
        result.mesh.k.push_back(index_map[n3]);
      }
    }

    std::string trim(const std::string &value)
    {
      size_t start =
        value.find_first_not_of(" \t\r\n");

      if (start == std::string::npos)
      {
        return "";
      }

      size_t end =
        value.find_last_not_of(" \t\r\n");

      return value.substr(
        start,
        end - start + 1);
    }

    int safe_stoi(const std::string &value)
    {
      try
      {
        return std::stoi(value);
      }
      catch (...)
      {
        return 0;
      }
    }
};

#endif