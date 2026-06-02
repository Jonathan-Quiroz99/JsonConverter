#pragma once

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include <cctype>

#include "classifiers/classification.h"
#include "../detectors/inspection_result.h"

// RuleEngine: rule-based scoring of PlotType and DataType from inspection info.
class RuleEngine
{
public:
    static ClassificationResult classify(
        const InspectionResult& insp,
        const std::string & /*filepath*/ = "",
        std::function<void(const std::string&)> log = nullptr)
    {
        auto logger = log ? log : [](const std::string&) {};
        ClassificationResult result;

        // Collect candidate lines: metadata_lines + metadata key/values
        std::vector<std::string> lines = insp.metadata_lines;
        for (const auto& kv : insp.metadata)
        {
            lines.push_back(kv.first + " " + kv.second);
        }

        // Normalize to uppercase
        for (auto& l : lines)
        {
            std::transform(l.begin(), l.end(), l.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        }

        std::map<PlotType, int> plot_score;
        std::map<DataType, int> data_score;

        auto match_and_add = [&](const std::string& token, PlotType p, int score, const std::string& ruleName)
            {
                for (const auto& l : lines)
                {
                    if (l.find(token) != std::string::npos)
                    {
                        plot_score[p] += score;
                        result.matched_rules.push_back(ruleName);
                    }
                }
            };

        auto match_and_add_data = [&](const std::string& token, DataType d, int score, const std::string& ruleName)
            {
                for (const auto& l : lines)
                {
                    if (l.find(token) != std::string::npos)
                    {
                        data_score[d] += score;
                        result.matched_rules.push_back(ruleName);
                    }
                }
            };

        // Plot rules (examples from requirement)
        match_and_add("ROWS ARE", PlotType::HEATMAP, 30, "ROWS ARE");
        match_and_add("COLUMNS ARE", PlotType::HEATMAP, 30, "COLUMNS ARE");
        match_and_add("$NODES", PlotType::MESH3D, 50, "$NODES");
        match_and_add("CONNECT", PlotType::MESH3D, 40, "CONNECTIVITY");
        match_and_add("$ELEMENTS", PlotType::MESH3D, 50, "$ELEMENTS");
        match_and_add("ELEMENT CONNECTIVITY", PlotType::MESH3D, 50, "ELEMENT CONNECTIVITY");
        match_and_add("GRID", PlotType::HEATMAP, 20, "GRID");
        match_and_add("MESH", PlotType::MESH3D, 20, "MESH");
        match_and_add("ROW 1", PlotType::HEATMAP, 25, "ROW 1");
        match_and_add("COL 1", PlotType::HEATMAP, 25, "COL 1");
        match_and_add("THROUGH", PlotType::HEATMAP, 10, "THROUGH");
        match_and_add("SURFACE NODE", PlotType::HEATMAP, 30, "SURFACE NODE");
        match_and_add("ELEMENT 1", PlotType::HEATMAP, 30, "ELEMENT");
        match_and_add("RADIAL", PlotType::HEATMAP, 40, "RADIAL NODE");
        match_and_add("CIRCUMFERENTIAL", PlotType::HEATMAP, 40, "CIRCUMFERENTIAL NODE");

        // =====================================================
        // SCATTER / TIME SERIES
        // =====================================================

        match_and_add(
            "TIMESTEP",
            PlotType::SCATTER,
            50,
            "TIMESTEP");

        match_and_add(
            "STRAIN ENERGY",
            PlotType::SCATTER,
            40,
            "STRAIN ENERGY");

        match_and_add(
            "FILTERED",
            PlotType::SCATTER,
            20,
            "FILTERED");

        match_and_add(
            "C1:",
            PlotType::SCATTER,
            20,
            "COLUMN DATA");

        match_and_add(
            "C2:",
            PlotType::SCATTER,
            20,
            "COLUMN DATA");

        // XYZ / coordinates heuristic
        for (const auto& l : lines)
        {
            if (
                l.find("X Y Z") != std::string::npos ||
                l.find("COORDINATES") != std::string::npos)
            {
                plot_score[PlotType::SURFACE] += 100;

                result.matched_rules.push_back(
                    "XYZ COORDINATES");

                break;
            }
        }

        // Data rules
        match_and_add_data("VON-MISES", DataType::STRESS, 50, "VON-MISES");
        match_and_add_data("VON MISES", DataType::STRESS, 50, "VON-MISES");
        match_and_add_data("STRESS", DataType::STRESS, 40, "STRESS");
        match_and_add_data("PRESSURE", DataType::PRESSURE, 50, "PRESSURE");
        match_and_add_data("TEMPERATURE", DataType::TEMPERATURE, 50, "TEMPERATURE");
        match_and_add_data("WEAR", DataType::WEAR, 40, "WEAR");
        match_and_add_data("FORCE", DataType::FORCE, 40, "FORCE");
        match_and_add_data("DISPLACEMENT", DataType::DISPLACEMENT, 40, "DISPLACEMENT");
        match_and_add_data("STRAIN ENERGY", DataType::STRAIN_ENERGY, 80, "STRAIN ENERGY");

        // Metadata hints
        for (const auto& kv : insp.metadata)
        {
            std::string key = kv.first;
            std::string val = kv.second;
            std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
            std::transform(val.begin(), val.end(), val.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
            if (key.find("ROWS") != std::string::npos || val.find("ROWS ARE") != std::string::npos)
            {
                plot_score[PlotType::HEATMAP] += 20;
                result.matched_rules.push_back("METADATA: ROWS");
            }
        }

        // =====================================================
        // STRUCTURE HEURISTICS
        // =====================================================

        // Typical XY series
        if (
            insp.cols == 2 &&
            insp.rows > 5)
        {
            plot_score[PlotType::SCATTER] += 100;

            result.matched_rules.push_back(
                "2 COLUMN SERIES");
        }

        // XYZ style data
        if (
            insp.cols == 3 &&
            insp.rows > 5)
        {
            plot_score[PlotType::SCATTER] += 50;

            result.matched_rules.push_back(
                "3 COLUMN SERIES");
        }

        // Matrix-like structure
        if (
            insp.cols > 5 &&
            insp.rows > 5)
        {
            plot_score[PlotType::HEATMAP] += 50;

            result.matched_rules.push_back(
                "MATRIX STRUCTURE");
        }

        // Determine best plot
        PlotType best_plot = PlotType::UNKNOWN;
        int best_plot_score = 0;
        int total_plot_score = 0;
        for (const auto& kv : plot_score)
        {
            total_plot_score += kv.second;
            if (kv.second > best_plot_score)
            {
                best_plot_score = kv.second;
                best_plot = kv.first;
            }
        }

        // Determine best data
        DataType best_data = DataType::UNKNOWN;
        int best_data_score = 0;
        int total_data_score = 0;
        for (const auto& kv : data_score)
        {
            total_data_score += kv.second;
            if (kv.second > best_data_score)
            {
                best_data_score = kv.second;
                best_data = kv.first;
            }
        }

        double plot_conf = 0.0;
        if (best_plot_score > 0 && total_plot_score > 0)
            plot_conf = static_cast<double>(best_plot_score) / static_cast<double>(total_plot_score);
        else if (best_plot_score > 0)
            plot_conf = 1.0;

        double data_conf = 0.0;
        if (best_data_score > 0 && total_data_score > 0)
            data_conf = static_cast<double>(best_data_score) / static_cast<double>(total_data_score);
        else if (best_data_score > 0)
            data_conf = 1.0;

        result.plot_type = best_plot;
        result.data_type = best_data;
        result.confidence = std::min(1.0, 0.7 * plot_conf + 0.3 * data_conf);

        logger("RuleEngine: plot best=" + std::to_string(static_cast<int>(best_plot)) + " score=" + std::to_string(best_plot_score));
        logger("RuleEngine: data best=" + std::to_string(static_cast<int>(best_data)) + " score=" + std::to_string(best_data_score));
        logger("RuleEngine: confidence=" + std::to_string(result.confidence));

        return result;
    }
};