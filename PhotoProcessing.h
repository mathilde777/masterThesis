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

        auto conversion_tolerance = 2.9;
        auto tolerance = 2.5;
        auto ref_2D_X = 1250;
        auto ref_2D_Y = 930;

        auto converted_x = ref_2D_X - (conversion_tolerance * x);
        auto converted_y = ref_2D_Y - (conversion_tolerance * y);

        // Calculate the bounding box coordinates
        int x1 = converted_x - width / 2;
        int y1 = converted_y - height / 2;

        // Ensure the bounding box is within the image boundaries
        x1 = std::max(0, x1);
        y1 = std::max(0, y1);

        height = height * tolerance;
        width = width * tolerance;

        int x2 = std::min(image.cols - 1, x1 + (width));
        int y2 = std::min(image.rows - 1, y1 + (height));

        std::cout << "x1: " << x1 << " y1: " << y1 << " x2: " << x2 << " y2: " << y2 << std::endl;
        std::cout << "width: " << width << " height: " << height << std::endl;

        // Crop the image to the bounding box
        cv::Rect roi(x1, y1, x2 - x1, y2 - y1);
        if (roi.width <= 0 || roi.height <= 0) {
            std::cerr << "Error: Invalid bounding box dimensions.\n";
            return ""; // Return empty string indicating failure
        }

        cv::Mat croppedImage = image(roi);


        std::string outputPath = "/home/user/windows-share/temp/cropped_image.jpg";
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

    // Function to find the latest .png file from Cropped Images directory
    std::optional<std::string> findLatestCroppedImage() {
        std::string directory = "/home/user/windows-share/temp";
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

    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm* now_tm = std::localtime(&now_time);

        std::ostringstream oss;
        oss << std::put_time(now_tm, "%m_%d_%H_%M");
        return oss.str();
    }

    // Function to store the cropped image in the database and also move it to specified directory
    // Parameter should be only the full path of the cropped image and boxID
    // So it renames a picture name to train_boxId.jpg and moves it to the specified directory
    void storeCroppedImage(const std::string& croppedImagePath, int boxId) {
        // Get the current timestamp
        std::string timestamp = getCurrentTimestamp();

        // Move the cropped image to the specified directory with the new name
        std::string newImagePath = "/home/user/windows-share/training/train_" + std::to_string(boxId) + "_" + timestamp + ".jpg";

        // Save the image to the specified directory
        fs::rename(croppedImagePath, newImagePath);

        // Print the new path
        std::cout << "Stored cropped image at: " << newImagePath << std::endl;
    }

};

#endif // PHOTOPROCESSING_H
