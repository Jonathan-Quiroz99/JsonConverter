#include "parser_factory.h"

#include "../parsers/pressure_conform_parser.h"
#include "../parsers/ring_forces_parser.h"
#include "../parsers/parser_face_wear.h"
#include "../parsers/PositionConformParser.h"
#include "../parsers/StrainEnergyConformParser.h"
#include "../parsers/FaceClearanceConformParser.h"
#include "../parsers/SurfaceAnimParser.h"
#include "../parsers/Scatter3dAnimParser.h"
#include <cctype> // Required for std::toupper


std::unique_ptr<BaseParser>
ParserFactory::create(
    const std::string& filepath)
{

    // Work on an uppercase copy for case-insensitive matching so we don't
    // attempt to modify the const input parameter.
    std::string upper_filepath = filepath;
    for (char& c : upper_filepath) {
        c = std::toupper(static_cast<unsigned char>(c));
    }

    if (upper_filepath.find("PRES_CONFORM") != std::string::npos)
    {
        return std::make_unique<
            PressureConformParser>(filepath);
    }

    if (upper_filepath.find("FACE_CLEARANCE_CONFORM") != std::string::npos)
    {
        return std::make_unique<
            FaceClearanceConformParser>(filepath);
    }

    if (upper_filepath.find("POS_CONFORM") != std::string::npos)
    {
        return std::make_unique<
            PositionConformParser>(filepath);
    }

    if (upper_filepath.find("STRAIN_ENERGY_CONFORM") != std::string::npos)
    {
        return std::make_unique<
            StrainEnergyConformParser>(filepath);
    }

    if (upper_filepath.find("RING_FORCES") != std::string::npos)
    {
        return std::make_unique<
            RingForcesParser>(
                filepath);
    }

    if (upper_filepath.find("_WEAR") != std::string::npos)
    {
        return std::make_unique<
            FaceWearParser>(
                filepath);
    }

    if (upper_filepath.find("POS_CYCLE") != std::string::npos)
    {
        return std::make_unique<
            Scatter3dAnimParser>(
                filepath);
	}

    if (upper_filepath.find("_CYCLE" ) != std::string::npos)
        {
        return std::make_unique<
            SurfaceAnimParser>(
                filepath);
	}

    return nullptr;
}