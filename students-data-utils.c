#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>

#include <jansson.h>

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

const char *analyzeString(const char *str)
{
    int containsNumbers = 0;
    int containsLetters = 0;

    for (int i = 0; str[i] != '\0'; i++)
    {
        // Check if the character is a digit
        if (isdigit((unsigned char)str[i]))
        {
            containsNumbers = 1;
        }
        // Check if the character is a letter
        else if (isalpha((unsigned char)str[i]))
        {
            containsLetters = 1;
        }
    }

    // Determine and print the result
    if (containsNumbers && containsLetters)
    {
        return "NUMBERS_AND_LETTERS";
    }
    else if (containsNumbers)
    {
        return "NUMBERS";
    }
    else if (containsLetters)
    {
        return "LETTERS";
    }
    else
    {
        // The string is empty or does not contain numbers or letters
        return "EMPTY";
    }
}

json_t *find_object_in_array(json_t *array, const char *key, const char *value)
{
    size_t index;
    json_t *object;

    json_array_foreach(array, index, object)
    {
        const char *object_value = json_string_value(json_object_get(object, key));
        if (object_value && strcmp(object_value, value) == 0)
        {
            return object;
        }
    }

    return NULL;
}

json_t *findObjectsInArray(json_t *array, const char *key, const char *value)
{
    json_t *matching_objects = json_array();
    size_t index;
    json_t *object;

    json_array_foreach(array, index, object)
    {
        const char *object_value = json_string_value(json_object_get(object, key));
        if (object_value && strcmp(object_value, value) == 0)
        {
            json_array_append(matching_objects, object);
        }
    }

    return matching_objects;
}

json_t *findObjectInArray(json_t *array, const char *key, const char *value)
{
    size_t index;
    json_t *object;

    json_array_foreach(array, index, object)
    {
        const char *object_value = json_string_value(json_object_get(object, key));
        if (object_value && strcmp(object_value, value) == 0)
        {
            return object;
        }
    }

    return NULL;
}

void printArrayOfObjects(json_t *array, bool appendRowNumber)
{
    size_t index;
    json_t *object;
    char *key;
    json_t *value;

    // Calculate the maximum width for each column
    int max_widths[100] = {0}; // Assuming a maximum of 100 columns
    json_array_foreach(array, index, object)
    {
        int col_index = 0;
        json_object_foreach(object, key, value)
        {
            const char *value_str = json_string_value(value);
            int length = value_str ? strlen(value_str) : 0;
            if (length > max_widths[col_index])
            {
                max_widths[col_index] = length;
            }
            col_index++;
        }
    }

    // Print the property names
    int col_index = 0;
    json_object_foreach(object, key, value)
    {
        char *str = (char *)malloc(max_widths[col_index] + 1);

        if (str == NULL)
        {
            fprintf(stderr, "Memory allocation error\n");
            pause();
            exit(EXIT_FAILURE);
        }

        strcpy(str, key);

        char prefixedStr[110] = "     ";

        if (appendRowNumber)
        {
            // Ensure prefixedStr is null-terminated
            prefixedStr[sizeof(prefixedStr) - 1] = '\0';

            // Check if there is enough space in str1 for concatenation
            if (strlen(prefixedStr) + strlen(str) < sizeof(prefixedStr))
            {
                strcat(prefixedStr, str);
            }
            else
            {
                fprintf(stderr, "Not enough space for concatenation\n");
                pause();
                return 1;
            }

            rjust(prefixedStr, max_widths[col_index], ' ');
            printf("%s  ", prefixedStr); // Two spaces to separate columns
        }
        else
        {
            rjust(str, max_widths[col_index], ' ');
            printf("%s  ", str);
        }

        free(str);
        col_index++;
    }
    printf("\n\n");

    // Print the property values
    json_array_foreach(array, index, object)
    {
        if (appendRowNumber)
        {
            printf("[%02d] ", (int)index + 1);
        }

        int col_index = 0;
        json_object_foreach(object, key, value)
        {
            const char *value_str = json_string_value(value);
            char *str = (char *)malloc(max_widths[col_index] + 1);
            if (str == NULL)
            {
                fprintf(stderr, "Memory allocation error\n");
                pause();
                exit(EXIT_FAILURE);
            }
            strcpy(str, value_str ? value_str : "");
            rjust(str, max_widths[col_index], ' ');
            printf("%s  ", str); // Two spaces to separate columns
            free(str);
            col_index++;
        }
        printf("\n");
    }
}

// Left-justification function
void ljust(char *str, int width, char padChar)
{
    int len = strlen(str);
    if (len < width)
    {
        int padding = width - len;
        strncat(str, &padChar, 1); // Append the padding character
        for (int i = 0; i < padding; ++i)
        {
            strncat(str, &padChar, 1);
        }
    }
}

// Right-justification function
void rjust(char *str, int width, char padChar)
{
    int len = strlen(str);
    if (len < width)
    {
        int padding = width - len;
        for (int i = 0; i < padding; ++i)
        {
            strncat(str, &padChar, 1);
        }
    }
}

// Center alignment function
void center(char *str, int width, char padChar)
{
    int len = strlen(str);
    if (len < width)
    {
        int padding = width - len;
        int leftPadding = padding / 2;
        int rightPadding = padding - leftPadding;
        for (int i = 0; i < leftPadding; ++i)
        {
            strncat(str, &padChar, 1);
        }
        strncat(str, " ", 1);
        for (int i = 0; i < rightPadding; ++i)
        {
            strncat(str, &padChar, 1);
        }
    }
}

void *createHyphenString(int numHyphens)
{
    // Allocate memory for the string
    char *hyphenString = (char *)malloc(numHyphens + 1); // One extra for the null terminator

    if (hyphenString == NULL)
    {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    // Fill the string with hyphens
    for (int i = 0; i < numHyphens; ++i)
    {
        hyphenString[i] = '-';
    }

    // Null-terminate the string
    hyphenString[numHyphens] = '\0';

    return hyphenString;
}

void printRawString(const char *str)
{
    for (int i = 0; str[i] != '\0'; ++i)
    {
        printf("%d ", str[i]);
    }
    printf("\n");
}

void removeNewlines(char *str)
{
    // Iterate through the string
    for (int i = 0; str[i]; ++i)
    {
        // Check for newline characters and remove them
        if (str[i] != '\n' && str[i] != '\r')
        {
            str[i] = str[i];
        }
        else
        {
            // If it's a newline, replace it with a null character
            str[i] = '\0';
        }
    }
}

int readFileStatic(const char *filename, char *buffer, size_t buffer_size)
{
    FILE *file = fopen(filename, "rb");

    if (file == NULL)
    {
        perror("Error opening file");
        return 0; // Return 0 to indicate failure
    }

    // Read at most (buffer_size - 1) bytes to leave room for the null terminator
    size_t bytes_read = fread(buffer, 1, buffer_size - 1, file);

    // Null-terminate the buffer
    buffer[bytes_read] = '\0';

    fclose(file);

    return 1; // Return 1 to indicate success
}

void removeObjectFromArray(json_t *array, const char *key, const char *value)
{
    size_t index;
    json_t *object;
    char *object_key;
    json_t *object_value;

    json_array_foreach(array, index, object)
    {
        object_value = json_object_get(object, key);
        const char *object_value_str = json_string_value(object_value);
        if (object_value_str && strcmp(object_value_str, value) == 0)
        {
            json_array_remove(array, index);
            break;
        }
    }
}

void modifyObject(json_t *object, const char *key, json_t *new_value)
{
    if (json_object_set(object, key, new_value))
    {
        fprintf(stderr, "error: unable to set value\n");
    }
}