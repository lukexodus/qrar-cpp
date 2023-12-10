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
#include "ReadBarcode.h"
#include "BarcodeFormat.h"
#include "DecodeHints.h"
#include <OpenXLSX.hpp>
#include <nlohmann/json.hpp>

#include "utils.hpp"

using namespace OpenXLSX;
namespace fs = std::filesystem;
using json = nlohmann::json;

int main()
{
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
			std::cout << "Please enter a filename for the excel file to be created. Press enter if you would like the default name [CCIS_ATTENDANCE.xlsx]\n> ";

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
			std::cout << "Please close the excel file then press enter to continue.\n> ";
			std::string continueProgram;
			std::getline(std::cin, continueProgram);
			continue;
		}
		else
		{
			bool excelFileIsOpen = false;

			for (const auto &file : fs::directory_iterator("."))
			{
				if (file.path().filename().string().find("~$") == 0 && endsWith(file.path().string(), ".xlsx"))
				{
					std::cout << "!!! An excel file is open in another window !!!" << std::endl;
					std::cout << "Please close the excel file/s then press enter to continue.\n> ";
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
				std::cout << "Enter which excel file to use (number)\n> ";
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

	std::vector<std::string> modes =
		{"AM Time In", "AM Time Out", "PM Time In", "PM Time Out"};

	int modeNum;
	std::cout << "\n\nSelect MODE\n[1] AM Time In\n[2] AM Time Out\n[3] PM Time In\n[4] PM Time Out\n> ";
	while (true)
	{
		std::cin >> modeNum;
		if (modeNum >= 1 && modeNum <= 4)
		{
			break;
		}
		std::cout << "Invalid input\n> ";
	}
	std::string mode = modes[modeNum - 1];

	// ************************ PHASE 2 ************************
	// Opens and retrieves the data from the backup [1] and the students data [2]

	// [1] The backup data (backup.json) serves as the temporary store for the attendance data
	// Structure:
	//		{
	//          attendance: {
	//              [date]: {
	//                  [course_and_section]: {
	//                      [mode]: {
	//                           [id]: [time]
	//                      }
	//                  }
	//              }
	//          }
	//      }

	// [2] The students data (students-data.json) is where the information associated with the IDs are derived from
	// This file is managed by students-data.exe
	// Structure:
	//      {
	//          [course_and_section]: [
	//              {
	//                  "name": [name],
	//                  "id": [id]
	//              }
	//          ]
	//      }

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

	std::string backupFilename = "backup.json";
	json backupData;

	if (isFileInCurrentDirectory(backupFilename))
	{
		std::ifstream f(backupFilename);
		backupData = json::parse(f);
	}
	else
	{
		// Creates/initializes the backup.json file
		std::ofstream outputFile(backupFilename);
		if (!outputFile.is_open())
		{
			std::cerr << "Error: Unable to create the JSON file." << std::endl;
		}

		backupData = {{"attendance", json::object()}};

		std::ofstream o(backupFilename);
		o << std::setw(4) << backupData << std::endl;
	}

	// ************************ PHASE 3 ************************
	// Converts the json into a list, for easier searching of data
	// This list contains objects, where each object contains all
	// information needed about one student

	// Structure:
	//      [
	//          {
	//              "name": [name],
	//              "id": [id],
	//              "course_and_section": [course_and_section]
	//          }
	//      ]

	json students = json::array();
	std::vector<std::string> sections;

	for (auto &sectionData : studentsData.items())
	{
		std::string section = sectionData.key();
		if (!isInVector(sections, section))
		{
			sections.push_back(section);
		}
		json studentObjectsArray = sectionData.value();
		for (const auto &studentObject : studentObjectsArray)
		{
			students.push_back({{"name", studentObject["name"]}, {"id", studentObject["id"]}, {"course_and_section", section}});
		}
	}

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
	// Gets the initial date to be checked with for date changes
	// "%a %Y%m%d" Date format (ex. "Tue 11-29-2023")
	std::string initialDate = datetimeStringByFormat("%a %m-%d-%Y");

	// Checks every 25 milliseconds if the
	// "Esc" (ASCII code 27) key is not pressed
	while (cv::waitKey(25) != 27)
	{
		// Captures frames from the camera
		cap >> image;

		// This configures the reader to be able to read barcodes also,
		// aside from QR codes
		auto hints = ZXing::DecodeHints().setFormats(ZXing::BarcodeFormat::Any);

		// ReadBarcodes (from ZXingOpenCV) extracts barcode info
		auto results = ReadBarcodes(image, hints);

		// Iterates every barcodes or QR codes scanned in an image
		for (auto &r : results)
		{
			// Draws the result to the webcam monitor (from ZXingOpenCV)
			DrawResult(image, r);

			std::string decodedID = r.text();
			std::string date = datetimeStringByFormat("%a %m-%d-%Y");
			// "%H:%M" Time format (ex. "15:45")
			std::string clockTime = datetimeStringByFormat("%H:%M");

			auto iterator = std::find_if(students.begin(), students.end(), [decodedID](const json &obj)
										 { return obj["id"] == decodedID; });

			std::string courseAndSection = (*iterator)["course_and_section"];

			// Initializes the properties if they are not initialized yet
			if (!backupData["attendance"].contains(date))
			{
				backupData["attendance"][date] = json::object();
			}
			if (!backupData["attendance"][date].contains(courseAndSection))
			{
				backupData["attendance"][date][courseAndSection] = json::object();
			}
			if (!backupData["attendance"][date][courseAndSection].contains(mode))
			{
				backupData["attendance"][date][courseAndSection][mode] = json::object();
			}

			// Check if the student is registered or not
			// If the student is registered, it stores the info (time)
			if (iterator != students.end())
			{
				std::string studentName = (*iterator)["name"];
				if (!backupData["attendance"][date][courseAndSection][mode].contains(decodedID))
				{
					backupData["attendance"][date][courseAndSection][mode][decodedID] = clockTime;
					std::cout << studentName << std::endl;
				}
			}
			else
			{
				std::cout << "Unregistered." << std::endl;
			}
		}

		// Title/header of the window
		cv::imshow("Attendance Tracking Program", image);
	}

	// ************************ PHASE 5 ************************
	// Stores the data to the backup file ("backup.json")

	std::cout << "Backing up data." << std::endl;
	std::ofstream outputFile(backupFilename);
	if (!outputFile.is_open())
	{
		std::cerr << "Error opening the file!" << std::endl;
		pause();
		return 1;
	}
	outputFile << backupData << std::endl;
	outputFile.close();

	// ************************ PHASE 6 ************************
	// Stores the necessary headers (dates, names, and IDs) to the excel file

	// Opens the excel file if it exists, otherwise initializes it
	XLDocument doc;
	if (noInitialFile)
	{
		doc.create(excelFilename);
	}
	else
	{
		doc.open(excelFilename);
	}

	XLWorkbook wbk = doc.workbook();

	for (const auto &section : sections)
	{
		// Creates the sheet only if it doesn't exists, otherwise uses it
		std::vector<std::string> sheetNames;
		for (const auto &sheetName : wbk.worksheetNames())
		{
			if (!isInVector(sheetNames, sheetName))
			{
				sheetNames.push_back(sheetName);
			}
		}
		if (!isInVector(sheetNames, section))
		{
			doc.workbook().addWorksheet(section);
		}
		auto wks = doc.workbook().worksheet(section);

		// Headers already written on the excel file before it is opened
		std::vector<std::string> alreadyWrittenIDs;
		std::vector<std::string> alreadyWrittenDates;

		// Headers written by the program during runtime
		std::vector<std::string> writtenIDs;
		std::vector<std::string> writtenDates;

		// Gets the dates already written to the sheet, and
		// Finds the first empty cell in the third row, starting from "C3"
		XLCell currentCell = wks.cell(XLCellReference("C3"));
		int currentColumnNum = 3;
		while (currentCell.value().type() != XLValueType::Empty)
		{
			XLCellValue cellValue = currentCell.value();
			std::string date = cellValue.get<std::string>();
			if (!isInVector(alreadyWrittenDates, date))
			{
				alreadyWrittenDates.push_back(date);
			}
			currentCell = wks.cell(XLCellReference(3, ++currentColumnNum));
		}
		int lastEmptyColumn = currentColumnNum;

		// Gets the IDs already written to the sheet, and
		// Finds the first empty cell in the first column, starting from "A5"
		XLCell currentCell2 = wks.cell(XLCellReference("A5"));
		int currentRowNum = 5;
		while (currentCell2.value().type() != XLValueType::Empty)
		{
			XLCellValue cellValue = currentCell2.value();
			std::string id = cellValue.get<std::string>();
			if (!isInVector(alreadyWrittenIDs, id))
			{
				alreadyWrittenIDs.push_back(id);
			}
			currentCell2 = wks.cell(XLCellReference(++currentRowNum, 1));
		}
		int lastEmptyRow = currentRowNum;

		// Writes the date not already written to the column headers (row 3)
		for (auto &recordsByDate : backupData["attendance"].items())
		{
			std::string date = recordsByDate.key();
			if (!isInVector(alreadyWrittenDates, date) && !isInVector(writtenDates, date))
			{
				int lastColumn = lastEmptyColumn + 4;
				for (int columnNum = lastEmptyColumn; columnNum < lastColumn; ++columnNum)
				{
					wks.cell(XLCellReference(3, columnNum)).value() = date;
					wks.cell(XLCellReference(4, columnNum)).value() = modes[((columnNum - 3) % 4)];
					lastEmptyColumn = columnNum + 1;
				}
				writtenDates.push_back(date);
			}

			for (auto &recordsByMode : backupData["attendance"][date][section].items())
			{
				std::string modeRecorded = recordsByMode.key();

				// Writes the IDs and names not already written to the IDs/names headers (columns 1 and 2 respectively)
				for (auto &recordsByID : backupData["attendance"][date][section][modeRecorded].items())
				{
					std::string id = recordsByID.key();
					auto iterator = std::find_if(students.begin(), students.end(), [id](const json &obj)
												 { return obj["id"] == id; });
					std::string sectionOfStudent = (*iterator)["course_and_section"];

					if (sectionOfStudent == section)
					{
						if (!isInVector(alreadyWrittenIDs, id) && !isInVector(writtenIDs, id))
						{
							std::string studentName = (*iterator)["name"];
							wks.cell(XLCellReference(lastEmptyRow, 1)).value() = id;
							wks.cell(XLCellReference(lastEmptyRow, 2)).value() = studentName;
							lastEmptyRow++;

							writtenIDs.push_back(id);
						}
					}
				}
			}
		}
	}

	doc.save();

	// ************************ PHASE 7 ************************
	// Stores the times recorded to the excel file

	wbk = doc.workbook();

	for (auto &recordsByDate : backupData["attendance"].items())
	{
		std::string date = recordsByDate.key();
		for (auto &recordsBySection : backupData["attendance"][date].items())
		{
			std::string section = recordsBySection.key();

			auto wks = wbk.worksheet(section);
			wbk.worksheet(section).setActive();

			for (auto &recordsByMode : backupData["attendance"][date][section].items())
			{
				std::string modeRecorded = recordsByMode.key();

				// Finds the column index to where the time info shall be placed for the student
				XLCell currentCell2 = wks.cell(XLCellReference("C3"));
				int currentColumn = 3;
				int columnIndex;
				while (true)
				{
					if (currentCell2.value().type() == XLValueType::Empty)
					{
						std::cout << "ERROR: Could not find the corresponding column coordinate for date " << date << std::endl;
						pause();
						return 1;
					}

					XLCellValue cellValue = currentCell2.value();
					std::string cellStringValue = cellValue.get<std::string>();

					if (cellStringValue == date)
					{
						columnIndex = currentColumn;
						break;
					}

					currentColumn++;
					currentCell2 = wks.cell(XLCellReference(3, currentColumn));
				}
				// Finds the appropriate column based on the mode
				int index = findIndex(modes, modeRecorded);
				columnIndex += index;

				for (auto &recordsByIDs : backupData["attendance"][date][section][modeRecorded].items())
				{
					std::string id = recordsByIDs.key();
					std::string time = recordsByIDs.value();

					// Finds the row index to where the time info shall be placed for the student
					XLCell currentCell = wks.cell(XLCellReference("A5"));
					int currentRow = 5;
					int rowIndex;
					while (true)
					{
						if (currentCell.value().type() == XLValueType::Empty)
						{
							std::cout << "ERROR: Could not find the corresponding row coordinate of the student" << id << std::endl;
							pause();
							return 1;
						}

						XLCellValue cellValue = currentCell.value();
						std::string cellStringValue = cellValue.get<std::string>();

						if (cellValue == id)
						{
							rowIndex = currentRow;
							break;
						}

						currentRow++;
						currentCell = wks.cell(XLCellReference(currentRow, 1));
					}

					// Stores the time info to the target cell
					wks.cell(XLCellReference(rowIndex, columnIndex)).value() = time;
				}
			}
		}
	}

	std::cout << "Saving excel file..." << std::endl;
	doc.save();
	doc.close();

	pause();

	return 0;
}
