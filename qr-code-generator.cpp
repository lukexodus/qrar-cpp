#include <iostream>
#include <filesystem>
#include <fstream>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include "BarcodeFormat.h"
#include "MultiFormatWriter.h"
#include "BitMatrix.h"
#include "BitMatrixIO.h"
#include <nlohmann/json.hpp>

#include "utils.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

cv::Mat BitMatrixToImage(const ZXing::BitMatrix &bitMatrix)
{
    cv::Mat image(bitMatrix.height(), bitMatrix.width(), CV_8UC1);
    for (int y = 0; y < bitMatrix.height(); ++y)
    {
        for (int x = 0; x < bitMatrix.width(); ++x)
        {
            image.at<uchar>(y, x) = bitMatrix.get(x, y) ? 0 : 255;
        }
    }
    return image;
}

void saveImageToFile(const cv::Mat &image, const std::string &filename)
{
    if (!cv::imwrite(filename, image))
    {
        throw std::runtime_error("Could not write image to file: " + filename);
    }
}

int main()
{
    std::string studentsDataFilename = "students-data.json";

    if (!isFileInCurrentDirectory(studentsDataFilename))
    {
        std::cout << "NO STUDENTS DATA (students-data.json) FOUND.\nPlease create one first before using this program. Use the students-data.exe program for this." << std::endl;
        std::cout << "Press Enter to exit...\n> ";
        std::cin.get();
        return 0;
    }

    std::ifstream f("students-data.json");
    json studentsData = json::parse(f);

    // Use the create_directory function to create the directory
    try
    {
        fs::create_directory("QR_Codes");
        std::cout << "Directory created successfully.\n";
    }
    catch (const fs::filesystem_error &ex)
    {
        std::cerr << "Error creating directory: " << ex.what() << '\n';
        pause();
        return 1;
    }

    for (auto &recordsBySection : studentsData.items())
    {
        std::string section = recordsBySection.key();

        std::string sectionDirectoryName = "QR_Codes/" + section;

        try
        {
            fs::create_directory(sectionDirectoryName);
            std::cout << "Directory created successfully.\n";
        }
        catch (const fs::filesystem_error &ex)
        {
            std::cerr << "Error creating directory: " << ex.what() << '\n';
            pause();
            return 1;
        }

        json studentObjects = recordsBySection.value();

        for (const auto &studentObject : studentObjects)
        {
            std::string studentName = studentObject["name"];
            std::string studentID = studentObject["id"];
            // Create a MultiFormatWriter for QR code
            ZXing::MultiFormatWriter writer(ZXing::BarcodeFormat::QRCode);

            // Encode the data
            ZXing::BitMatrix bitMatrix = writer.encode(studentID, 300, 300);

            // Convert the BitMatrix to an image
            cv::Mat image = BitMatrixToImage(bitMatrix);

            // Save the image or process it further
            saveImageToFile(image, sectionDirectoryName + "/" + studentName + " (" + studentID + ").png");
        }
    }

    pause();
    return 0;
}