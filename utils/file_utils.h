#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>
#include <fstream>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

namespace file_utils {

    inline bool file_exists(const std::string& path) {
        try {
            return fs::exists(path) && fs::is_regular_file(path);
        } catch (...) {
            return false;
        }
    }

    inline std::string get_output_path(const std::string& input_path) {
        std::string directory;
        std::string filename;

        size_t pos = input_path.find_last_of("/\\");
        if (pos != std::string::npos) {
            directory = input_path.substr(0, pos);
            filename = input_path.substr(pos + 1);
        }
        else {
            directory = ".";
            filename = input_path;
        }

        std::string stem = filename;
        size_t ext_pos = stem.find_last_of('.');
        if (ext_pos != std::string::npos) {
            stem = stem.substr(0, ext_pos);
        }

        std::string output_dir = directory + "\\json_files";

        // Create directory (recursively)
        try {
            fs::create_directories(output_dir);
        } catch (...) {
            // ignore
        }

        return output_dir + "\\" + stem + ".json";
    }

    inline bool save_json(const std::string& json_str, const std::string& output_path) {
        std::ofstream file(output_path);
        if (!file.is_open()) {
            return false;
        }

        file << json_str;
        file.close();
        return true;
    }

    inline std::vector<std::string> find_files_to_process(const std::string& directory) {
        std::vector<std::string> files;

        try {
            for (const auto& entry : fs::directory_iterator(directory)) {
                if (!entry.is_regular_file()) continue;
                std::string fname = entry.path().filename().string();

                if (fname == "." || fname == "..") continue;

                // Skip hidden files or specific extensions
                if (!fname.empty() && fname[0] == '.') continue;
                std::string ext = entry.path().extension().string();
                if (ext != ".py" && ext != ".json" && fname != "json_files") {
                    files.push_back(entry.path().string());
                }
            }
        } catch (...) {
            // ignore directory iteration errors
        }

        return files;
    }

} // namespace file_utils

#endif // FILE_UTILS_H