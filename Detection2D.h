
#ifndef DETECTION2D_H
#define DETECTION2D_H

#include <memory>
#include <string>
#include <vector>

struct DetectionResult {
    int label;
    std::pair<double, double> dimensions;
    std::pair<double, double> center;

};
std::shared_ptr<std::vector<DetectionResult>> run2D(const char* file_path, int index);
std::string getBuffer(const char* file_path, int index);
std::string escapeJsonString(const std::string& input);
std::vector<DetectionResult> getLabels(const std::string& buffer, int index);

#endif // DETECTION2D_H

