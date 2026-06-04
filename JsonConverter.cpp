#include <iostream>
#include <fstream>
#include <memory>
#include <string>

#include "factory/parser_factory.h"
#include "builders/json_builder.h"

int main()
{
    std::string inputFile;

    std::cout
        << "Enter file path:\n";

    std::getline(
        std::cin,
        inputFile);

    auto parser =
        ParserFactory::create(
            inputFile);

    if (!parser)
    {
        std::cout
            << "\nNo parser available for:\n"
            << inputFile
            << std::endl;

        return 1;
    }

    PlotData data =
        parser->parse();

    std::string json =
        JsonBuilder::build(data);

    std::string outputFile =
        inputFile;

    size_t pos =
        outputFile.find_last_of('.');

    if (pos != std::string::npos)
    {
        outputFile =
            outputFile.substr(
                0,
                pos);
    }

    outputFile += ".json";

    std::ofstream out(
        outputFile);

    if (!out.is_open())
    {
        std::cout
            << "\nFailed creating output file\n";

        return 1;
    }

    out << json;

    out.close();

    std::cout
        << "\nJSON created successfully:\n"
        << outputFile
        << std::endl;

    return 0;
}