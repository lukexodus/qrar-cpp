#include <jansson.h>

char **getFilenames(const char *dir_path);

void freeArrayOfStrings(char **arrayOfStrings);

void pause();

void readLine(char *buffer, size_t bufferSize, FILE *stream);

const char *analyzeString(const char *str);

json_t *findObjectsInArray(json_t *array, const char *key, const char *value);

json_t *findObjectInArray(json_t *array, const char *key, const char *value);

void printArrayOfObjects(json_t *array);

void ljust(char *str, int width, char padChar);

void rjust(char *str, int width, char padChar);

void center(char *str, int width, char padChar);

char *createHyphenString(int numHyphens);

void printRawString(const char *str);

void removeNewlines(char *str);

int readFileStatic(const char *filename, char *buffer, size_t buffer_size);

void removeObjectFromArray(json_t *array, const char *key, const char *value);