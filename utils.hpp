#include <string>
#include <vector>

bool isFileInCurrentDirectory(const std::string &filename);

bool isFileInCurrentDirectory(const char *filename);

bool endsWith(const std::string &fullString, const std::string &ending);

std::vector<std::string> getExcelFiles(std::string path);

std::string datetimeStringByFormat(const char *format);

template <typename T>
bool isInArray(const T arr[], int size, const T &value);

template <typename T>
bool isInVector(const std::vector<T> &vec, const T &value);

#include "utils.tpp"

void pause();

int findIndex(const std::vector<std::string> &vec, const std::string &searchString);