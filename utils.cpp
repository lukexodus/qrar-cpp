#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <ctime>

#include <opencv2/opencv.hpp>
#include "ZXingOpenCV.h"
#include "BarcodeFormat.h"
#include <OpenXLSX.hpp>
#include <nlohmann/json.hpp>

#include "utils.hpp"

namespace fs = std::filesystem;
using namespace cv;
using namespace OpenXLSX;
using json = nlohmann::json;

bool isFileInCurrentDirectory(const std::string &filename)
{
    // Get the current working directory
    fs::path currentPath = fs::current_path();

    // Append the filename to the current path
    fs::path filePath = currentPath / filename;

    // Check if the file exists
    return fs::exists(filePath) && fs::is_regular_file(filePath);
}

bool isFileInCurrentDirectory(const char *filename)
{
    // Get the current working directory
    fs::path currentPath = fs::current_path();

    // Append the filename to the current path
    fs::path filePath = currentPath / filename;

    // Check if the file exists
    return fs::exists(filePath) && fs::is_regular_file(filePath);
}

// Function that checks whether a string ends in a specific string or not
bool endsWith(const std::string &fullString, const std::string &ending)
{
    if (fullString.length() >= ending.length())
    {
        // `compare` string method returns 0 if the two substrings are equal
        return (fullString.compare(fullString.length() - ending.length(), ending.length(), ending) == 0);
    }
    else
    {
        return false;
    }
}

// Function that returns an array of strings of all the excel files in a directory
std::vector<std::string> getExcelFiles(std::string path)
{
    std::vector<std::string> excelFiles;

    try
    {
        for (const auto &filename : fs::directory_iterator("."))
        {
            std::string extensionName = ".xlsx";
            if (endsWith(filename.path().string(), extensionName))
            {
                excelFiles.push_back(filename.path().string());
            }
        }
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return excelFiles;
}

std::string datetimeStringByFormat(const char *format)
{
    // Get the current time point
    auto now = std::chrono::system_clock::now();

    // Convert the current time point to a time_t
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

    // Convert time_t to a tm structure (broken down time)
    std::tm *localTime = std::localtime(&currentTime);

    // Format the date as a string
    std::ostringstream oss;
    oss << std::put_time(localTime, format);

    // Returns the formatted date as a string
    return oss.str();
}

void pause()
{
    std::cout << "Press Enter to exit...";
    // int string;
    // std::cin >> string;
    std::cin.get();
}

int findIndex(const std::vector<std::string> &vec, const std::string &searchString)
{
    auto it = std::find(vec.begin(), vec.end(), searchString);

    if (it != vec.end())
    {
        return std::distance(vec.begin(), it);
    }
    else
    {
        return -1; // Not found
    }
}