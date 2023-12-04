#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <csignal>
#include <iomanip>
#include <chrono>
#include <ctime>

#include <opencv2/opencv.hpp>
#include "ZXingOpenCV.h"
#include <OpenXLSX.hpp>
#include <nlohmann/json.hpp>

#include "utils.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

// Function to handle the interrupt signal (Ctrl+C)
// void signalHandler(void);
// void signalHandler(void) {
//     std::cout << "Interrupt signal received.\n";

//     // exit(signum);
// }

// struct student
// {
// 	std::string name;
// 	std::string id;
// 	std::string course_and_section;
// };

// void from_json(const json &j, student &s)
// {
// 	j.at("name").get_to(s.name);
// 	j.at("id").get_to(s.id);
// 	j.at("course_and_section").get_to(s.course_and_section);
// }

int main()
{
	// signal(SIGINT, signalHandler);

	// ************************ PHASE 1 ************************
	// Gets the filename of the excel file to be used

	std::string programDirectory = ".";

	std::string excelFilename;
	bool noInitialFile = false;

	while (true)
	{
		std::vector<std::string> excelFiles = getExcelFiles(programDirectory);
		if (excelFiles.empty())
		{
			noInitialFile = true;
			std::cout << "No xlsx files found in the directory." << std::endl;
			std::cout << "Please enter a filename for the excel file to be created. Press enter if you would like the default name [CCIS_ATTENDANCE.xlsx]" << std::endl;

			std::getline(std::cin, excelFilename);

			if (excelFilename.empty())
			{
				excelFilename = "CCIS_ATTENDANCE.xlsx";
			}
			if (!endsWith(excelFilename, ".xlsx"))
			{
				excelFilename += ".xlsx";
			}
			break;
		}
		else if (excelFiles.size() == 1)
		{
			excelFilename = excelFiles[0];
			std::cout << "Found " << excelFilename << ". Now using this as the local database." << std::endl;
			std::cout << "Exit the program (Ctrl+C) if this is not the excel file you want." << std::endl;
			break;
		}
		else if (excelFiles.size() == 2 &&
				 (excelFiles[0].find("~$") == 0 || excelFiles[1].find("~$") == 0))
		{
			std::cout << "!!! The excel file is open in another window !!!" << std::endl;
			std::cout << "Please close the excel file then press enter to continue." << std::endl;
			std::string continueProgram;
			std::getline(std::cin, continueProgram);
			continue;
		}
		else
		{
			bool excelFileIsOpen = false;
			;
			for (const auto &file : fs::directory_iterator("."))
			{
				if (file.path().filename().string().find("~$") == 0 && endsWith(file.path().string(), ".xlsx"))
				{
					std::cout << "!!! An excel file is open in another window !!!" << std::endl;
					std::cout << "Please close the excel file/s then press enter to continue." << std::endl;
					std::string continueProgram;
					std::getline(std::cin, continueProgram);

					for (const auto &filename : fs::directory_iterator("."))
					{
						if (filename.path().filename().string().find("~$") == 0 && endsWith(filename.path().string(), ".xlsx"))
						{
							excelFileIsOpen = true;
							break;
						}
					}
				}
			}

			if (excelFileIsOpen)
			{
				excelFiles.clear();
				continue;
			}
			std::cout << "Multiple xlsx files found in the directory." << std::endl;
			int excelFilesNum = excelFiles.size();
			for (int i = 0; i < excelFilesNum; ++i)
			{
				std::cout << "[" << i + 1 << "] " << excelFiles[i] << std::endl;
			}
			int number;
			while (true)
			{
				std::cout << "Enter which excel file to use (number) > ";
				std::cin >> number;
				// If inputted number is out of range, ask again
				if (number < 1 || number > excelFiles.size())
				{
					std::cout << "Number invalid." << std::endl;
					continue;
				}
				excelFilename = excelFiles[number - 1];
				break;
			}
			std::cout << "Now using  " << excelFilename << " as the local database." << std::endl;

			break;
		}
	}

	// ************************ PHASE 2 ************************
	// Opens the backup data [1] and the students data [2]
	//
	// [1] The backup data (backup.json) serves as the temporary store for the attendance data
	// Structure:
	//		{
	//			attendance: {
	//				[date]: {
	//					[name]: [time]
	//              }
	//			}
	//		}

	std::string studentsDataFilename = "students-data.json";

	if (!isFileInCurrentDirectory(studentsDataFilename))
	{
		std::cout << "NO STUDENTS DATA (students-data.json) FOUND.\nPlease create one first before using this program. Use the students-data.exe program for this." << std::endl;
		std::cout << "Press Enter to exit...";
		std::cin.get();
		return 0;
	}

	std::ifstream f("students-data.json");
	json studentsData = json::parse(f);

	std::string backupFilename = "backup.json";
	json backupData;

	if (isFileInCurrentDirectory(backupFilename))
	{
		std::ifstream f(backupFilename);
		backupData = json::parse(f);
	}
	else
	{
		// Creates/initializes the cache.json file
		std::ofstream outputFile(backupFilename);
		if (!outputFile.is_open())
		{
			std::cerr << "Error: Unable to create the JSON file." << std::endl;
		}

		backupData = {{"atttendance", json::object()}};
		std::ofstream o(backupFilename);
		o << std::setw(4) << backupData << std::endl;
	}

	// ************************ PHASE 3 ************************
	// Converts the json into a list, for easier searching of data

	json students = json::array();

	for (auto &sectionData : studentsData.items())
	{
		std::string section = sectionData.key();
		json studentObjectsArray = sectionData.value();
		for (const auto &studentObject : studentObjectsArray)
		{
			students.push_back({{"name", studentObject["name"]}, {"id", studentObject["id"]}, {"course_and_section", section}});
		}
	}

	// std::vector<student> students = students.get<std::vector<student>>();

	// ************************ PHASE 4 ************************
	// Opens the webcam to scan QR codes

	cv::namedWindow("Attendance Tracking Program");

	cv::Mat image;
	cv::VideoCapture cap(0);

	if (!cap.isOpened())
	{
		std::cout << "Could not open camera" << std::endl;
		return 0;
	}
	else
	{
		// Gets the initial date to be checked with for date changes
		// "%a %Y%m%d" Date format (ex. "Tue 11-29-2023")
		std::string initialDate = datetimeStringByFormat("%a %m-%d-%Y");

		// Checks every 25 milliseconds if the
		// "Esc" (ASCII code 27) key is not pressed
		while (cv::waitKey(25) != 27)
		{
			// Captures frames from the camera
			cap >> image;

			// ReadBarcodes (from ZXingOpenCV) extracts barcode info
			auto results = ReadBarcodes(image);

			// Iterates every barcodes or QR codes scanned in an image
			for (auto &r : results)
			{
				// Draws the result to the webcam monitor (from ZXingOpenCV)
				DrawResult(image, r);

				std::string decodedData = r.text();
				std::string date = datetimeStringByFormat("%a %m-%d-%Y");
				// "%H:%M" Time format (ex. "15:45")
				std::string clockTime = datetimeStringByFormat("%H:%M");
				if (!backupData.contains(date))
				{
					backupData[date] = json::object();
				}

				auto it = std::find_if(students.begin(), students.end(), [decodedData](const json &obj)
									   { return obj["id"] == decodedData; });

				// Check if the student object was found
				if (it != students.end())
				{
					std::string studentName = (*it)["name"];
					if (!backupData[date].contains(studentName))
					{
						backupData[date][studentName] = clockTime;
						std::cout << (*it)["name"] << std::endl;
					}
				}
				else
				{
					std::cout << "Unregistered." << std::endl;
				}
			}

			cv::imshow("Attendance Tracking Program", image);
		}
	}

	// XLDocument doc;
	// doc.create("Spreadsheet.xlsx");
	// auto wks = doc.workbook().worksheet("Sheet1");
	// wks.cell("A1").value() = "Hello, OpenXLSX!";
	// doc.save();

	std::cout << "Press Enter to exit...";
	std::cin.get();

	return 0;
}
