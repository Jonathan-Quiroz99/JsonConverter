#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

namespace file_utils
{
    /*
    ============================================================
    FILE EXISTS
    ============================================================
    */

    inline bool file_exists(const std::string& path)
    {
        try
        {
            return
                fs::exists(path) &&
                fs::is_regular_file(path);
        }
        catch (...)
        {
            return false;
        }
    }

    /*
    ============================================================
    READ FILE
    ============================================================
    */

    inline bool read_all_lines(
        const std::string& path,
        std::vector<std::string>& lines)
    {
        std::ifstream file(path);

        if (!file.is_open())
        {
            return false;
        }

        std::string line;

        while (std::getline(file, line))
        {
            lines.push_back(line);
        }

        file.close();

        return true;
    }

    /*
    ============================================================
    OUTPUT PATH
    Example:
        input:
            C:\data\stress.txt

        output:
            C:\data\json_files\stress.json
    ============================================================
    */

    inline std::string get_output_path(
        const std::string& input_path)
    {
        try
        {
            fs::path input(input_path);

            fs::path directory =
                input.parent_path();

            fs::path stem =
                input.stem();

            fs::path output_dir =
                directory / "json_files";

            /*
            CREATE DIRECTORY
            */

            fs::create_directories(output_dir);

            fs::path output_file =
                output_dir / (stem.string() + ".json");

            return output_file.string();
        }
        catch (...)
        {
            return "output.json";
        }
    }

    /*
    ============================================================
    SAVE JSON
    ============================================================
    */

    inline bool save_json(
        const std::string& json_str,
        const std::string& output_path)
    {
        try
        {
            std::ofstream file(
                output_path,
                std::ios::out | std::ios::trunc);

            if (!file.is_open())
            {
                return false;
            }

            file << json_str;

            file.close();

            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    /*
    ============================================================
    FIND FILES
    ============================================================
    */

    inline std::vector<std::string>
        find_files_to_process(
            const std::string& directory)
    {
        std::vector<std::string> files;

        try
        {
            for (
                const auto& entry :
                fs::directory_iterator(directory))
            {
                if (!entry.is_regular_file())
                {
                    continue;
                }

                std::string filename =
                    entry.path().filename().string();

                std::string extension =
                    entry.path().extension().string();

                /*
                SKIP HIDDEN FILES
                */

                if (
                    !filename.empty() &&
                    filename[0] == '.')
                {
                    continue;
                }

                /*
                SKIP GENERATED FILES
                */

                if (
                    extension == ".json" ||
                    extension == ".py")
                {
                    continue;
                }

                files.push_back(
                    entry.path().string());
            }
        }
        catch (...)
        {
        }

        return files;
    }

    /*
    ============================================================
    LOWERCASE
    ============================================================
    */

    inline std::string to_lower(
        std::string value)
    {
        std::transform(
            value.begin(),
            value.end(),
            value.begin(),
            [](unsigned char c)
            {
                return std::tolower(c);
            });

        return value;
    }

} // namespace file_utils