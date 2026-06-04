#include "parser_factory.h"

#include "../parsers/pressure_conform_parser.h"
#include "../parsers/ring_forces_parser.h"
#include "../parsers/parser_face_wear.h"
#include "../parsers/PositionConformParser.h"
#include "../parsers/StrainEnergyConformParser.h"
#include "../parsers/FaceClearanceConformParser.h"

std::unique_ptr<BaseParser>
ParserFactory::create(
    const std::string& filepath)
{
    if (filepath.find("PRES_CONFORM") != std::string::npos)
    {
        return std::make_unique<
            PressureConformParser>(filepath);
    }

    if (filepath.find("FACE_CLEARANCE_CONFORM") != std::string::npos)
    {
        return std::make_unique<
            FaceClearanceConformParser>(filepath);
    }

    if (filepath.find("POS_CONFORM") != std::string::npos)
    {
        return std::make_unique<
            PositionConformParser>(filepath);
    }

    if (filepath.find("STRAIN_ENERGY_CONFORM") != std::string::npos)
    {
        return std::make_unique<
            StrainEnergyConformParser>(filepath);
    }

    if (
        filepath.find("RING_FORCES")
        != std::string::npos)
    {
        return std::make_unique<
            RingForcesParser>(
                filepath);
    }

    if (
        filepath.find("_WEAR")
        != std::string::npos)
    {
        return std::make_unique<
            FaceWearParser>(
                filepath);
    }

    return nullptr;
}