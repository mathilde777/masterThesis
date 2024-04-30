// detection2D.cpp

#include "detection2D.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <sstream>

std::shared_ptr<std::vector<DetectionResult>> run2D(const char* file_path) {
    std::string buffer = getBuffer(file_path);
    std::vector<DetectionResult> result = getLabels(buffer);
    return std::make_shared<std::vector<DetectionResult>>(result);
}

std::string getBuffer(const char* file_path) {
    // Create the command string with the file path variable
    char command[200];
    snprintf(command, sizeof(command), "curl -X POST http://localhost:5001/predict -H \"Content-Type: application/json\" -d \"{\\\"img_path\\\": \\\"%s\\\"}\" 2>&1", file_path);

    FILE *pipe = popen(command, "r");
    if (pipe == nullptr) {
        perror("popen failed");
        return "";
    }

    std::string buffer;
    char temp_buffer[128];
    while (fgets(temp_buffer, sizeof(temp_buffer), pipe) != nullptr) {
        buffer += temp_buffer;
    }
    pclose(pipe);

    return buffer;
}

std::vector<DetectionResult> getLabels(const std::string& buffer) {
    std::vector<DetectionResult> results;

    // Find the position of "box" in the buffer
    size_t pos = 0;
    while ((pos = buffer.find("\"box\"", pos)) != std::string::npos) {
        // Extract the relevant information for each detection result
        DetectionResult result;

        // Parse dimensions
        size_t dim_start = buffer.find("[", pos);
        size_t dim_end = buffer.find("]", dim_start);
        std::string dim_str = buffer.substr(dim_start + 1, dim_end - dim_start - 1);
        sscanf(dim_str.c_str(), "%lf,%lf", &result.dimensions.first, &result.dimensions.second);

        // Parse center
        size_t center_start = buffer.find("[", dim_end);
        size_t center_end = buffer.find("]", center_start);
        std::string center_str = buffer.substr(center_start + 1, center_end - center_start - 1);
        sscanf(center_str.c_str(), "%lf,%lf", &result.center.first, &result.center.second);

        size_t label_start = buffer.find("\"label\"", center_end);
        size_t label_quote_start = buffer.find("\"", label_start + 7);
        size_t label_quote_end = buffer.find("\"", label_quote_start + 1);
        std::string label_str = buffer.substr(label_quote_start + 1, label_quote_end - label_quote_start - 1);

        // Find the position of the numeric part in the label string
        size_t numeric_start = label_str.find_first_of("0123456789");

        // Extract the numeric part of the label
        int label = std::stoi(label_str.substr(numeric_start));

        // Add the DetectionResult to the results vector with the integer label
        result.label = label;
        results.push_back(result);

        // Move to the next detection result
        pos = label_quote_end;
    }

    return results;
}

