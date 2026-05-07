#ifndef PARSER_2D_H
#define PARSER_2D_H

#include "base_parser.h"

using namespace std;

class Parser2D : public BaseParser {
public:
    Parser2D(const string& path, function<void(const string&)> callback = nullptr)
        : BaseParser(path, callback) {
    }

    ParsedData parse() override {
        log("  Parsing as 2D data");

        ParsedData result;
        result.filename = filename;
        result.type = FileType::TIME_SERIES_2D;

        ifstream file(filepath);
        if (!file.is_open()) {
            log("  Error opening file");
            return result;
        }

        vector<string> lines;
        string line;
        while (getline(file, line)) {
            if (!line.empty()) {
                lines.push_back(line);
            }
        }
        file.close();

        // Detect data type from headers
        string data_type = "generic_2d";
        vector<string> headers;
        vector<string> units;

        for (size_t i = 0; i < min(lines.size(), size_t(10)); i++) {
            string upper_line = lines[i];
            transform(upper_line.begin(), upper_line.end(), upper_line.begin(), ::toupper);

            if (upper_line.find("PISTON POSITION") != string::npos) {
                data_type = "piston_position";
                log("  Detected type: Piston Position");
            }
            else if (upper_line.find("PISTON VELOCITY") != string::npos) {
                data_type = "piston_velocity";
                log("  Detected type: Piston Velocity");
            }
            else if (upper_line.find("PISTON ACCELERATION") != string::npos) {
                data_type = "piston_acceleration";
                log("  Detected type: Piston Acceleration");
            }
        }

        // Extract numeric data
        result.numeric_data = extract_numeric_data(true);
        result.data_type = data_type;
        result.headers = headers;
        result.units = units;

        if (!result.numeric_data.empty()) {
            log("  Numeric rows: " + to_string(result.numeric_data.size()));
            log("  Columns per row: " + to_string(result.numeric_data[0].size()));
        }
        else {
            log("  WARNING: No numeric data extracted");
        }

        return result;
    }
};

#endif // PARSER_2D_H