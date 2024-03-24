
#ifndef DETECTION2D_H
#define DETECTION2D_H

#include <memory>
#include <string>
#include <vector>

struct DetectionResult {
    std::string label;
    std::vector<int> points;
};

std::unique_ptr<DetectionResult> run2D(const char* file_path);
std::string getBuffer(const char* file_path);
DetectionResult getLabel(const std::string& buffer);

#endif // DETECTION2D_H

