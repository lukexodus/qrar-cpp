#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "students-data-utils.h"

char **getFilenames(const char *dir_path)
{
    HANDLE hFind;
    WIN32_FIND_DATA FindFileData;
    char **filenames = NULL;
    int count = 0;

    hFind = FindFirstFile(dir_path, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "error: could not open directory %s\n", dir_path);
        return NULL;
    }

    do
    {
        filenames = realloc(filenames, sizeof(char *) * (count + 1));
        filenames[count] = _strdup(FindFileData.cFileName);
        count++;
    } while (FindNextFile(hFind, &FindFileData) != 0);

    FindClose(hFind);

    filenames = realloc(filenames, sizeof(char *) * (count + 1));
    filenames[count] = NULL;

    return filenames;
}

void freeArrayOfStrings(char **arrayOfStrings)
{
    char **p = arrayOfStrings;
    while (*p != NULL)
    {
        free(*p);
        p++;
    }
    free(arrayOfStrings);
}

void pause()
{
    printf("\nPress Enter to exit...\n");
    getchar();
}

void readLine(char *buffer, size_t bufferSize, FILE *stream)
{
    if (fgets(buffer, bufferSize, stream) == NULL)
    {
        fprintf(stderr, "Error reading input\n");
        pause();
        return 1;
    }

    // Remove trailing newline, if present
    size_t length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n')
    {
        buffer[length - 1] = '\0';
    }
}
