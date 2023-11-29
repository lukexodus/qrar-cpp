#include <string>
#include <vector>

bool isFileInCurrentDirectory(const std::string& filename);

bool isFileInCurrentDirectory(const char* filename);

bool endsWith(const std::string& fullString, const std::string& ending);

std::vector<std::string> getExcelFiles(std::string path);

std::string datetimeStringByFormat(const char* format);