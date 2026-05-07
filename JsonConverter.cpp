#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include "detectors/file_detector.h"
#include "parsers/parser_generic.h"  // NEW: generic parser
#include "parsers/parser_2d.h"
#include "parsers/parser_3d.h"
#include "parsers/parser_gmsh.h"
#include "utils/file_utils.h"
#include "builders/json_builder.h"

using namespace std;
namespace fs = std::filesystem;

void log_message(const string& msg) {
    cout << "[LOG] " << msg << endl;
}

bool process_single_file(const string& filepath) {
    log_message("Processing: " + filepath);

    if (!file_utils::file_exists(filepath)) {
        log_message("  Error: File not found: " + filepath);
        return false;
    }

    // Try to detect file type
    FileType type = FileDetector::detect(filepath, log_message);
    log_message("Detected type: " + to_string(static_cast<int>(type)));

    ParsedData parsed_data;

    // Use specific parser based on type, fallback to generic parser
    switch (type) {
    case FileType::TIME_SERIES_2D:
    case FileType::CORRELATION_2D:
        log_message("Using 2D parser...");
        parsed_data = Parser2D(filepath, log_message).parse();
        break;
    case FileType::GRID_2D:
    case FileType::MESH_3D_SURFACE:
    case FileType::RING_WEAR:
        log_message("Using 3D parser...");
        parsed_data = Parser3D(filepath, log_message).parse();
        break;
    case FileType::MESH_3D_VOLUME:
        log_message("Using Gmsh parser...");
        parsed_data = ParserGmsh(filepath, log_message).parse();
        break;
    default:
        log_message("Using generic/adaptive parser...");
        ParserGeneric generic_parser(filepath, log_message);
        parsed_data = generic_parser.parse();
        break;
    }

    // If no data was extracted, try generic parser as fallback
    if (parsed_data.numeric_data.empty() && parsed_data.sections.empty() && !parsed_data.has_mesh()) {
        log_message("No data extracted with specific parser, trying generic parser...");
        ParserGeneric generic_parser(filepath, log_message);
        parsed_data = generic_parser.parse();
    }

    log_message("Building JSON structure...");
    string json_output = JsonBuilder::build(parsed_data, log_message);

    if (json_output.empty()) {
        log_message("  Error: Failed to build JSON output");
        return false;
    }

    string output_path = file_utils::get_output_path(filepath);
    if (!file_utils::save_json(json_output, output_path)) {
        log_message("  Error: Could not save JSON file");
        return false;
    }

    log_message("JSON saved to: " + output_path);
    cout << "\n[OK] Success! Output: " << output_path << endl;
    return true;
}

int main() {
    string input;
    cout << "=== Simple File to JSON Converter ===" << endl;
    cout << "Enter file path: ";
    getline(cin, input);

    // Trim
    while (!input.empty() && isspace(static_cast<unsigned char>(input.front()))) input.erase(0, 1);
    while (!input.empty() && isspace(static_cast<unsigned char>(input.back()))) input.pop_back();
    if (!input.empty() && input.front() == '"' && input.back() == '"') {
        input = input.substr(1, input.length() - 2);
    }

    if (input.empty()) {
        cerr << "Error: No path provided!" << endl;
        return 1;
    }

    try {
        fs::path p = fs::absolute(fs::path(input));
        input = p.string();
    }
    catch (...) {}

    try {
        fs::path p(input);
        if (!fs::exists(p)) {
            cerr << "Error: Path does not exist: " << input << endl;
            return 1;
        }
        if (fs::is_directory(p)) {
            cerr << "Error: Provided path is a directory. Please provide a single file path." << endl;
            return 1;
        }
    }
    catch (const std::exception& ex) {
        cerr << "Error: Exception while accessing path: " << ex.what() << endl;
        return 1;
    }

    if (process_single_file(input)) return 0;
    return 1;
}