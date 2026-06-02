#pragma once
#include <map>
#include <functional>
#include <memory>
#include <string>

#include "../classifiers/classification.h"
#include "../detectors/inspection_result.h"
#include "../parsers/base_parser.h"
#include "../parsers/parser_heatmap.h"
#include "../parsers/parser_surface.h"
#include "../parsers/parser_3d.h"
#include "../parsers/parser_2d.h"

// ParserRegistry: map PlotType -> factory
class ParserRegistry
{
public:
    using ParserFactory = std::function<std::unique_ptr<BaseParser>(const std::string&, const InspectionResult&, std::function<void(const std::string&)>)>;

    static ParserRegistry& instance()
    {
        static ParserRegistry reg;
        return reg;
    }

    void register_parser(PlotType pt, ParserFactory factory)
    {
        factories[pt] = factory;
    }

    bool has_parser(PlotType pt) const
    {
        return factories.find(pt) != factories.end();
    }

    ParserFactory get(PlotType pt) const
    {
        auto it = factories.find(pt);
        if (it != factories.end()) return it->second;
        return nullptr;
    }

private:
    ParserRegistry()
    {
        register_parser(PlotType::HEATMAP, [](const std::string& path, const InspectionResult& insp, std::function<void(const std::string&)> cb) {
            return std::unique_ptr<BaseParser>(new ParserHeatmap(path, insp, cb));
            });

        register_parser(PlotType::SURFACE, [](const std::string& path, const InspectionResult& insp, std::function<void(const std::string&)> cb) {
            return std::unique_ptr<BaseParser>(new ParserSurface(path, insp, cb));
            });

        register_parser(PlotType::MESH3D, [](const std::string& path, const InspectionResult& insp, std::function<void(const std::string&)> cb) {
            return std::unique_ptr<BaseParser>(new Parser3D(path, insp, cb));
            });

        register_parser(PlotType::SCATTER, [](const std::string& path, const InspectionResult& insp, std::function<void(const std::string&)> cb) {
            return std::unique_ptr<BaseParser>(new Parser2D(path, insp, cb));
            });
    }

    std::map<PlotType, ParserFactory> factories;
};