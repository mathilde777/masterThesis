#ifndef RESULT_H
#define RESULT_H

#include <string>
#include "clusters.h"

class Result {
public:

    Result(const ClusterInfo& info, float confidence) : info(info), confidence(confidence) {}

    ClusterInfo getLabel() const { return info; }
    float getConfidence() const { return confidence; }

    void setLabel(const ClusterInfo& newLabel) { info = newLabel; }
    void setConfidence(float newConfidence) { confidence = newConfidence; }

private:
    float confidence;
    ClusterInfo info;
};

#endif
