#ifndef PHOTOPROCESSING_H
#define PHOTOPROCESSING_H

#include <opencv2/opencv.hpp>
#include <optional>
#include <iostream>
#include <filesystem>
#include <string>

//#include <opencv>
namespace fs = std::filesystem;
class PhotoProcessor {

public:
    // Function to crop the image to a specified box
    std::string cropToBox(const std::string& imagePath, int x, int y, int width, int height) {
        // Load the image
        cv::Mat image = cv::imread(imagePath);
        if (image.empty()) {
            std::cerr << "Error: Unable to load image " << imagePath << std::endl;
            return ""; // Return empty string indicating failure
        }
        auto conversion_tolerance = 2.7;
        auto ref_2D_X = 1250;
        auto ref_2D_Y = 930;

        auto converted_x = ref_2D_X - (conversion_tolerance*x);
        auto converted_y = ref_2D_Y - (conversion_tolerance*y);

        // Calculate the bounding box coordinates
        int x1 = converted_x - width / 2;
        int y1 = converted_y - height / 2;

        // Ensure the bounding box is within the image boundaries
        x1 = std::max(0, x1);
        y1 = std::max(0, y1);
        int x2 = std::min(image.cols - 1, x1 + width);
        int y2 = std::min(image.rows - 1, y1 + height);

        // Crop the image to the bounding box
        cv::Rect roi(x1, y1, x2 - x1, y2 - y1);
        cv::Mat croppedImage = image(roi);

        // Save the cropped image as JPEG
        std::string outputPath = "/home/user/windows-share/cropped_image.jpg";
        cv::imwrite(outputPath, croppedImage);

        return outputPath;
    }

    // Function to find the latest .ply file in a given directory
    std::optional<std::string> findLatestPlyFile(const std::string& directory) {
        std::optional<std::string> latestFile;
        auto latestTime = fs::file_time_type::min();

        // Check if the directory exists and is a directory
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Provided path is not a directory or doesn't exist.\n";
            return latestFile;
        }

        // Iterate through each item in the directory
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".ply") {
                auto currentFileTime = fs::last_write_time(entry);
                if (currentFileTime > latestTime) {
                    latestTime = currentFileTime;
                    latestFile = entry.path(); // Store the full path instead of just the filename
                }
            }
        }

        return latestFile;
    }

    //Function to find the latest .png file in a given directory
    std::optional<std::string> findLatestPngFile(const std::string& directory) {
        namespace fs = std::filesystem;
        std::optional<std::string> latestFile;
        auto latestTime = fs::file_time_type::min();

        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Provided path is not a directory or doesn't exist.\n";
            return latestFile;
        }

        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".jpg") {
                auto currentFileTime = fs::last_write_time(entry);
                if (currentFileTime > latestTime) {
                    latestTime = currentFileTime;
                    latestFile = entry.path();
                }
            }
        }

        return latestFile;
    }

};

#endif // PHOTOPROCESSING_H
