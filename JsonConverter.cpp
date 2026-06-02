//cpp JsonConverter.cpp
#include <string>
#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>
#include <iomanip>
#include <iostream>

#include "detectors/file_detector.h"
#include "detectors/inspection_result.h"

#include "parsers/parser_generic.h"
#include "parsers/parser_2d.h"
#include "parsers/parser_surface.h"
#include "parsers/parser_3d.h"
#include "parsers/parser_gmsh.h"

#include "utils/file_utils.h"
#include "builders/json_builder.h"

#include "classifiers/rule_engine.h"
#include "registry/parser_registry.h"

using namespace std;
namespace fs = std::filesystem;

void log_message(const string& msg)
{
    cout << "[LOG] " << msg << endl;
}

static std::string plotTypeToString(PlotType p)
{
    switch (p)
    {
    case PlotType::SCATTER: return "scatter";
    case PlotType::HEATMAP: return "heatmap";
    case PlotType::SURFACE: return "surface";
    case PlotType::MESH3D: return "mesh3d";
    case PlotType::CONTOUR: return "contour";
    case PlotType::HISTOGRAM: return "histogram";
    default: return "unknown";
    }
}

static std::string dataTypeToString(DataType d)
{
    switch (d)
    {
    case DataType::STRESS: return "stress";
    case DataType::PRESSURE: return "pressure";
    case DataType::TEMPERATURE: return "temperature";
    case DataType::WEAR: return "wear";
    case DataType::FORCE: return "force";
    case DataType::DISPLACEMENT: return "displacement";
    default: return "unknown";
    }
}

static bool save_diagnostic_json_for(const std::string& input_path, const ClassificationResult& classification)
{
    try
    {
        fs::path p(input_path);
        fs::path out = p.parent_path() / (p.stem().string() + ".diagnostic.json");

        std::ostringstream diag;
        diag << "{\n";
        diag << "  \"detected_plot_type\": \"" << plotTypeToString(classification.plot_type) << "\",\n";
        diag << "  \"detected_data_type\": \"" << dataTypeToString(classification.data_type) << "\",\n";
        diag << "  \"confidence\": " << std::fixed << std::setprecision(2) << classification.confidence << ",\n";
        diag << "  \"matched_rules\": [\n";
        for (size_t i = 0; i < classification.matched_rules.size(); ++i)
        {
            diag << "    \"" << classification.matched_rules[i] << "\"";
            if (i + 1 < classification.matched_rules.size()) diag << ",";
            diag << "\n";
        }
        diag << "  ],\n";
        diag << "  \"parser_available\": " << (classification.parser_available ? "true" : "false") << "\n";
        diag << "}\n";

        if (!file_utils::save_json(diag.str(), out.string()))
        {
            log_message("Failed saving diagnostic JSON to: " + out.string());
            return false;
        }

        log_message("Diagnostic JSON saved: " + out.string());
        return true;
    }
    catch (...)
    {
        log_message("Exception saving diagnostic JSON");
        return false;
    }
}

bool process_single_file(const string& filepath)
{
    log_message("Processing: " + filepath);

    if (!file_utils::file_exists(filepath))
    {
        log_message("File not found");
        return false;
    }

    InspectionResult inspection =
        FileInspector::inspect(
            filepath,
            log_message);

    log_message("Inspector header lines: " + std::to_string(inspection.header_lines));
    log_message("Inspector metadata lines: " + std::to_string(inspection.metadata_lines.size()));

    ClassificationResult classification =
        RuleEngine::classify(inspection, filepath, log_message);

    log_message("Detected plot type: " + plotTypeToString(classification.plot_type));
    log_message("Detected data type: " + dataTypeToString(classification.data_type));
    log_message("Confidence: " + std::to_string(classification.confidence));

    if (!classification.matched_rules.empty())
    {
        log_message("Matched rules:");
        for (const auto& r : classification.matched_rules)
        {
            log_message("  - " + r);
        }
    }

    ParserRegistry& registry = ParserRegistry::instance();

    classification.parser_available = registry.has_parser(classification.plot_type);

    log_message(std::string("Parser available: ") + (classification.parser_available ? "yes" : "no"));

    if (!classification.parser_available)
    {
        // Save diagnostic next to input file and do not fallback
        save_diagnostic_json_for(filepath, classification);
        return false;
    }

    auto factory = registry.get(classification.plot_type);
    if (!factory)
    {
        log_message("No factory found for detected plot type");
        return false;
    }

    std::unique_ptr<BaseParser> parser = factory(filepath, inspection, log_message);

    if (!parser)
    {
        log_message("Failed to create parser instance");
        return false;
    }

    ParsedData parsed_data = parser->parse();

    parsed_data.plot_type = plotTypeToString(classification.plot_type);

    if (parsed_data.data_type.empty() || parsed_data.data_type == "generic_data")
    {
        parsed_data.data_type = dataTypeToString(classification.data_type);
    }

    if (
        parsed_data.numeric_data.empty() &&
        parsed_data.sections.empty() &&
        !parsed_data.has_mesh())
    {
        log_message("No data extracted");
        return false;
    }

    log_message("Building JSON...");

    string json_output =
        JsonBuilder::build(
            parsed_data,
            log_message);

    if (json_output.empty())
    {
        log_message("Failed to build JSON");
        return false;
    }

    string output_path =
        file_utils::get_output_path(filepath);

    if (!file_utils::save_json(json_output, output_path))
    {
        log_message("Failed saving JSON to: " + output_path);
        return false;
    }

    log_message("JSON saved: " + output_path);

    cout << endl;
    cout << "[OK] Success" << endl;

    return true;
}

int main(int argc, char** argv)
{
    string input;

    if (argc > 1)
    {
        input = argv[1];
        // trim whitespace
        while (!input.empty() && isspace(static_cast<unsigned char>(input.front())))
            input.erase(0, 1);
        while (!input.empty() && isspace(static_cast<unsigned char>(input.back())))
            input.pop_back();
    }
    else
    {
        cout << "=== JSON Converter ===" << endl;
        cout << "Enter file path: ";
        getline(cin, input);

        while (
            !input.empty() &&
            isspace(static_cast<unsigned char>(input.front())))
        {
            input.erase(0, 1);
        }

        while (
            !input.empty() &&
            isspace(static_cast<unsigned char>(input.back())))
        {
            input.pop_back();
        }

        if (
            !input.empty() &&
            input.front() == '"' &&
            input.back() == '"')
        {
            input = input.substr(1, input.length() - 2);
        }
    }

    if (input.empty())
    {
        cerr << "No path provided" << endl;
        return 1;
    }

    try
    {
        fs::path p = fs::absolute(fs::path(input));
        input = p.string();
    }
    catch (...)
    {
    }

    if (!process_single_file(input))
    {
        return 1;
    }

    return 0;
}