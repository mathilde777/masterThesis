// detection2D.cpp

#include "detection2D.h"
#include <iostream>
#include <cstdio>
#include <cstring> // for strstr

std::unique_ptr<DetectionResult> run2D(const char* file_path) {
    std::string buffer = getBuffer(file_path);
    DetectionResult result = getLabel(buffer);
    return std::make_unique<DetectionResult>(result);
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

DetectionResult getLabel(const std::string& buffer) {

    DetectionResult result;

    // Find the position of "label" in the buffer
    size_t labelPos = buffer.find("label");
    if (labelPos != std::string::npos) {

        size_t quotePos = buffer.find("\"", labelPos + 7);
        if (quotePos != std::string::npos) {
            result.label = buffer.substr(quotePos + 1, buffer.find("\"", quotePos + 1) - quotePos - 1);
            std::cout << "Label: " << result.label << std::endl;
            size_t labelNumPos = result.label.find_last_of("0123456789");
            if (labelNumPos != std::string::npos) {
                result.label = result.label.substr(labelNumPos);
            }
        }
    }

    // Find the position of "dimensions" in the buffer
    size_t dimPos = buffer.find("dimensions");
    if (dimPos != std::string::npos) {

        size_t quotePos = buffer.find("\"", dimPos + 13);
        if (quotePos != std::string::npos) {

            result.dimensions = buffer.substr(quotePos + 1, buffer.find("\"", quotePos + 1) - quotePos - 1);
            std::cout << "Dimensions: " << result.dimensions << std::endl;
        }
    }

    // Find the position of "center" in the buffer
    size_t centerPos = buffer.find("center");
    if (centerPos != std::string::npos) {
        size_t bracketPos = buffer.find("[", centerPos + 8);
        if (bracketPos != std::string::npos) {
            size_t commaPos = buffer.find(",", bracketPos + 1);
            if (commaPos != std::string::npos) {
                result.center.first = std::stoi(buffer.substr(bracketPos + 1, commaPos - bracketPos - 1));
                size_t endBracketPos = buffer.find("]", commaPos + 1);
                if (endBracketPos != std::string::npos) {
                    result.center.second = std::stoi(buffer.substr(commaPos + 1, endBracketPos - commaPos - 1));
                    std::cout << "Center: (" << result.center.first << ", " << result.center.second << ")" << std::endl;
                }
            }
        }
    }
    return result;
}

