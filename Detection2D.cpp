// detection2D.cpp

#include "Detection2D.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <sstream>

std::shared_ptr<std::vector<DetectionResult>> run2D(const char* file_path, int index) {
    std::string buffer = getBuffer(file_path, index);
    std::vector<DetectionResult> result = getLabels(buffer,index);
    return std::make_shared<std::vector<DetectionResult>>(result);
}

// Correctly escapes file paths for inclusion in a JSON string.
std::string escapeJsonString(const std::string& input) {
    std::string output;
    output.reserve(input.length() + 20); // More space for potential escape characters
    for (char c : input) {
        switch (c) {
        case '\\': output += "\\\\"; break;
        case '"':  output += "\\\""; break;
        default:   output += c; break;
        }
    }
    return output;
}

std::string getBuffer(const char* file_path, int index) {
    char command[512];
    std::string escapedPath = escapeJsonString(file_path);

    if (index == 0) {
        std::cout << "Running predict" << std::endl;
        snprintf(command, sizeof(command), "curl -X POST http://localhost:5001/predict -H \"Content-Type: application/json\" -d '{\"img_path\": \"%s\"}' 2>&1", escapedPath.c_str());
    } else if (index == 1) {
        std::cout << "Running predict cropped box" << std::endl;
        snprintf(command, sizeof(command), "curl -X POST http://localhost:5001/predict_box -H \"Content-Type: application/json\" -d '{\"img_path\": \"%s\"}' 2>&1", escapedPath.c_str());
    }

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

std::vector<DetectionResult> getLabels(const std::string& buffer, int index) {
    std::vector<DetectionResult> results;

    // Find the position of "box" in the buffer
    if(index == 0)
    {
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

    }
    else{

        DetectionResult result;
        // Find the position of the "result" key
        size_t pos = buffer.find("\"result\"");
        if (pos == std::string::npos) {
            std::cerr << "Key not found";
            return results;
        }

        // Find the position of the label value
        size_t label_start = buffer.find("\"", pos + 9);
        size_t label_end = buffer.find("\"", label_start + 1);
        if (label_start == std::string::npos || label_end == std::string::npos) {
            std::cerr << "Label not found";
           return results;
        }

        // Extract the label value
        std::string label_str = buffer.substr(label_start + 1, label_end - label_start - 1);

        // Find the position of the first numeric character
        size_t numeric_start = label_str.find_first_of("0123456789");
        if (numeric_start == std::string::npos) {
            std::cerr << "Numeric part not found";
            return results;
        }

        // Extract the numeric part
        std::string numeric_str = label_str.substr(numeric_start);

        // Convert the numeric part to an integer
        int label = std::stoi(numeric_str);
        result.label = label;
        results.push_back(result);


        /**
         size_t pos = buffer.find("Box");
        if (pos != std::string::npos) {
            // Extract the substring starting from the position after "Box"
            std::string boxNumber = buffer.substr(pos + 3);
            // Convert the substring to an integer
            int number = std::stoi(boxNumber);
            DetectionResult detectionResult;
            detectionResult.label = number;

            results.push_back(detectionResult);

        }
**/

    }

    return results;
}



