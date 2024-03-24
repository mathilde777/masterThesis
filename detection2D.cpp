// detection2D.cpp

#include "detection2D.h"
#include <iostream>
#include <sstream>
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

    // Execute the command using popen
    FILE *pipe = popen(command, "r");
    if (pipe == nullptr) {
        perror("popen failed");
        return "";
    }

    // Read the output of the command into a buffer
    std::string buffer;
    char temp_buffer[128];
    while (fgets(temp_buffer, sizeof(temp_buffer), pipe) != nullptr) {
        buffer += temp_buffer;
    }

    // Close the pipe
    pclose(pipe);

    return buffer;
}

DetectionResult getLabel(const std::string& buffer) {
    // Parsing buffer to extract label and points
    DetectionResult result;

    // Find the position of "label" in the buffer
    size_t labelPos = buffer.find("label");
    if (labelPos != std::string::npos) {
        // Find the position of the first quote after "label"
        size_t quotePos = buffer.find("\"", labelPos + 7); // adding 7 to skip "label":
        if (quotePos != std::string::npos) {
            // Extract the label between the quotes
            result.label = buffer.substr(quotePos + 1, buffer.find("\"", quotePos + 1) - quotePos - 1);
            std::cout << "Label: " << result.label << std::endl;
        }
    }

    // TODO: Extract points from the buffer

    return result;
}
