#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <jansson.h>

#include "students-data-utils.h"

#define ESCAPE_KEY 27
#define MAX_STR_LENGTH 100

int main()
{

    // [1] Checks if students-data.json already exists in the current directory

    char **filenames = getFilenames("*.*");
    if (filenames == NULL)
    {
        fprintf(stderr, "Error getting filenames\n");
        return 0;
    }
    bool jsonFileExists = false;

    for (int i = 0; filenames[i] != NULL; ++i)
    {
        if (strcmp("students-data.json", filenames[i]) == 0)
        {
            jsonFileExists = true;
            break;
        }
    }
    freeArrayOfStrings(filenames);

    json_t *jsonObject = json_object();
    int sectionNum = 0;
    char **sections = NULL;

    // [2] Retrieves the necessary data

    if (!jsonFileExists)
    {
        // [2A] INITIALIZES DATA

        // [2A.1] Get the sections of the college from the user

        char buffer[MAX_STR_LENGTH]; // 99 characters maximum string length

        printf("Input all sections of the college, for example \"BSCS 1B\", \"BSIT 4A\", etc\n");
        printf("Press Enter if done\n");

        while (true)
        {
            printf("Enter section name: ");

            readLine(buffer, sizeof(buffer), stdin);
            // or use `scanf("%s", &buffer);` but fgets is safer

            // Remove the trailing newline character, if present
            size_t length = strlen(buffer);
            if (length > 0 && buffer[length - 1] == '\n')
            {
                buffer[length - 1] = '\0';
            }

            // Check if the entered string is empty (just Enter key pressed)
            if (buffer[0] == '\0')
            {
                break;
            }

            // Dynamic allocation of strings
            sections = realloc(sections, (sectionNum + 1) * sizeof(char *));
            if (sections == NULL)
            {
                fprintf(stderr, "Memory allocation error\n");
                pause();
                return 1;
            }
            sections[sectionNum] = strdup(buffer);
            if (sections[sectionNum] == NULL)
            {
                fprintf(stderr, "Memory allocation error\n");
                pause();
                return 1;
            }
            sectionNum++;
        }

        // [2A.2] Writes sections to json file

        // Creates students-data.json file
        FILE *file = fopen("students-data.json", "w");
        if (file == NULL)
        {
            fprintf(stderr, "error: could not open file\n");
            pause();
            return 1;
        }

        // Creates the json object
        for (int i = 0; i < sectionNum; ++i)
        {
            json_object_set_new(jsonObject, sections[i], json_object());
        }

        // Write the json to file
        char *jsonStringValue = json_dumps(jsonObject, JSON_ENCODE_ANY);
        if (fputs(jsonStringValue, file) == EOF)
        { // EOF is end-of-file i.e. an error occured
            fprintf(stderr, "error: could not write to file\n");
            pause();
            return 1;
        }

        fclose(file);
    }
    else
    {
        // [2B] RETRIEVES PREVIOUSLY STORED DATA

        // [2B.1] Get data from students-data.json file

        // Opens students-data.json file
        FILE *file = fopen("students-data.json", "r");
        if (file == NULL)
        {
            fprintf(stderr, "error: could not open file\n");
            pause();
            return 1;
        }

        // Reads its contents
        char jsonString[2048];
        readLine(jsonString, sizeof(jsonString), file);

        // Convert content to json
        json_error_t error;
        jsonObject = json_loads(jsonString, 0, &error);
        if (!jsonObject)
        {
            fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
            pause();
            return 1;
        }

        // Gets all sections registered
        const char *key;
        json_t *value;
        json_object_foreach(jsonObject, key, value)
        {
            sections = realloc(sections, (sectionNum + 1) * sizeof(char *));
            if (sections == NULL)
            {
                fprintf(stderr, "Memory allocation error\n");
                pause();
                return 1;
            }
            sections[sectionNum] = strdup(key);
            if (sections[sectionNum] == NULL)
            {
                fprintf(stderr, "Memory allocation error\n");
                pause();
                return 1;
            }
            sectionNum++;
        }

        fclose(file);
    }

    // [3] Goes to edit mode

    printf("------------------ Sections ------------------\n");
    for (int i = 0; sections[i] != NULL; ++i)
    {
        printf("[%d] %s\n", i + 1, sections[i]);
    }
    printf("\nSelect section > ");

    pause();

    return 0;
}
