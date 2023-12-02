#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <jansson.h>

#include "students-data-utils.h"

#define ESCAPE_KEY 27

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
    int sectionsNum = 0;
    char **sections = NULL;

    // [2] Retrieves the necessary data

    if (!jsonFileExists)
    {
        // [2A] INITIALIZES DATA

        // [2A.1] Get the sections of the college from the user

        char buffer[100]; // 99 characters maximum string length

        printf("Input all sections of the college, for example \"BSCS 1B\", \"BSIT 4A\", etc\n");
        printf("Press Enter if done\n");

        while (true)
        {
            printf("Enter section name: ");

            readLine(buffer, sizeof(buffer), stdin);
            // or use `scanf("%s", &buffer);` but fgets is safer

            // Check if the entered string is empty (just Enter key pressed)
            if (buffer[0] == '\0')
            {
                break;
            }

            // Dynamic allocation of strings
            sections = realloc(sections, (sectionsNum + 1) * sizeof(char *));
            if (sections == NULL)
            {
                fprintf(stderr, "Memory allocation error\n");
                pause();
                return 1;
            }
            sections[sectionsNum] = strdup(buffer);
            if (sections[sectionsNum] == NULL)
            {
                fprintf(stderr, "Memory allocation error\n");
                pause();
                return 1;
            }
            sectionsNum++;
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
        for (int i = 0; i < sectionsNum; ++i)
        {
            json_object_set_new(jsonObject, sections[i], json_array());
        }

        // Write the json to file
        char *jsonStringValue = json_dumps(jsonObject, JSON_INDENT(2));
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

        // Opens students-data.json file and reads its contents
        char jsonString[1024];
        if (!readFileStatic("students-data.json", jsonString, sizeof(jsonString)))
        {
            fprintf(stderr, "Error reading input\n");
            pause();
            return 1;
        }

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
            sections = realloc(sections, (sectionsNum + 1) * sizeof(char *));
            if (sections == NULL)
            {
                fprintf(stderr, "Memory allocation error\n");
                pause();
                return 1;
            }
            sections[sectionsNum] = strdup(key);
            if (sections[sectionsNum] == NULL)
            {
                fprintf(stderr, "Memory allocation error\n");
                pause();
                return 1;
            }
            sectionsNum++;
        }
    }

    // [3] Goes to edit mode

    bool invalidInput = false;

    char *chosenSection;
    // Main loop
    while (true)
    {
        // Displays the list of sections
        system("cls");

        char dashPadding[] = "------------------";
        printf("%s Sections %s\n\n", dashPadding, dashPadding);
        if (sectionsNum > 0)
        {
            for (int i = 0; i < sectionsNum; ++i)
            {
                printf("[%d] %s\n", i + 1, sections[i]);
            }
        }
        else
        {
            printf("No sections configured yet");
        }
        printf("\n----------------------------------------------\n");

        // Instructions
        printf("\nSelect section to edit\n\nOR\n\n[a] Add section\n[r] Remove section\n[q] (Quit) Save changes and exit program\n\n");
        if (invalidInput)
        {
            printf("Invalid input.\n> ");
            invalidInput = false;
        }
        else
        {
            printf("> ");
        }

        // [3.1] Inputs desired action

        char response[10];
        readLine(response, sizeof(response), stdin);

        // [3.2] Action

        if (analyzeString(response) == "NUMBERS")
        {
            // [3.2A] If the user wants to select a section

            // atoi ASCII to integer i.e. converts the text to integer type
            int num = atoi(response);

            // If within range
            if (num > 0 && num <= sectionsNum)
            {
                bool continueMainLoop = false;
                bool invalidInput2 = false;

                // [3.3] Students list loop
                while (true)
                {
                    // Gets the list of students in the specified section
                    chosenSection = sections[num - 1];
                    json_t *studentsArrayInSection = json_object_get(jsonObject, chosenSection);
                    size_t studentsInsectionsNum = json_array_size(studentsArrayInSection);

                    // Checks whether there are registered students in the section
                    bool isThereStudentsInSection = studentsInsectionsNum != 0;

                    // Displays the header of the list
                    system("cls");
                    char formattedString[20];
                    sprintf(formattedString, "%s %s %s", dashPadding, chosenSection, dashPadding);
                    printf("%s\n\n", formattedString);

                    if (isThereStudentsInSection)
                    {
                        // Prints an aligned data table of the list of students
                        printArrayOfObjects(studentsArrayInSection, true);
                    }
                    else
                    {
                        printf("No students registered yet");
                    }

                    // Footer line
                    printf("\n\n%s\n", createHyphenString(strlen(formattedString)));

                    printf("\nSelect which student to edit\n\nOR\n\n[a] Add student\n[r] Remove student\n[b] Go back\n\n");
                    if (invalidInput2)
                    {
                        printf("Invalid input.\n> ");
                        invalidInput2 = false;
                    }
                    else
                    {
                        printf("> ");
                    }

                    // Inputs desired action
                    char response[10];
                    readLine(response, sizeof(response), stdin);

                    if (analyzeString(response) == "NUMBERS")
                    {
                        int num = atoi(response);
                        if (num > 0 && num <= studentsInsectionsNum)
                        {
                            json_t *studentObject = json_array_get(studentsArrayInSection, num - 1);

                            char newName[100];
                            printf("Enter the new name [%s] > ", json_string_value(json_object_get(studentObject, "name")));
                            readLine(newName, sizeof(newName), stdin);
                            if (strlen(newName) != 0)
                            {
                                json_t *newNameVal = json_string(newName);
                                modifyObject(studentObject, "name", newNameVal);
                                json_decref(newNameVal);
                            }

                            char newID[10];
                            printf("Enter the new ID [%s] > ", json_string_value(json_object_get(studentObject, "id")));
                            readLine(newID, sizeof(newID), stdin);
                            if (strlen(newID) != 0)
                            {
                                json_t *newIDVal = json_string(newID);
                                modifyObject(studentObject, "id", newIDVal);
                                json_decref(newIDVal);
                            }
                            continue;
                        }
                        else
                        {
                            invalidInput2 = true;
                            continue;
                        }
                    }
                    else if (analyzeString(response) == "LETTERS")
                    {
                        if (strcmp(response, "a") == 0)
                        {
                            // Adds student

                            // Inputs student name and id
                            char *name[100], *id[10];
                            printf("\nEnter student name > ");
                            readLine(name, sizeof(name), stdin);
                            printf("\nEnter student ID > ");
                            readLine(id, sizeof(id), stdin);

                            // Creates the student object and appends to data
                            json_t *newStudentObject = json_object();
                            json_object_set_new(newStudentObject, "name", json_string(name));
                            json_object_set_new(newStudentObject, "id", json_string(id));
                            if (!json_is_array(studentsArrayInSection))
                            {
                                fprintf(stderr, "error: the property 'studentsArrayInSection' is not an array\n");
                                pause();
                                return 1;
                            }
                            json_array_append(studentsArrayInSection, newStudentObject);

                            json_decref(newStudentObject); // JSON Decrease Reference: frees memory
                            continue;
                        }

                        else if (strcmp(response, "r") == 0)
                        {
                            // Removes a student

                            while (true)
                            {
                                // Inputs id
                                char *id[10];
                                printf("\nEnter the ID of the student to be removed > ");
                                readLine(id, sizeof(id), stdin);

                                if (findObjectInArray(studentsArrayInSection, "id", id))
                                {
                                    removeObjectFromArray(studentsArrayInSection, "id", id);
                                    break;
                                }
                                else
                                {
                                    printf("Student with the inputted ID not found\n");
                                }
                            }
                            continue;
                        }

                        else if (strcmp(response, "b") == 0)
                        {
                            // Go back
                            continueMainLoop = true;
                            break;
                        }
                        else
                        {
                            invalidInput2 = true;
                            continue;
                        }
                    }
                }
                if (continueMainLoop)
                {
                    continueMainLoop = false;
                    continue;
                }
            }
            else
            {
                invalidInput = true;
                continue;
            }
        }

        // [3.2B] If the user wants to add/remove sections or wants to quit

        else if (analyzeString(response) == "LETTERS")
        {
            if (strcmp(response, "a") == 0)
            {
                // Adds section

                char response[100];
                printf("Enter the name of the section to be added > ");
                readLine(response, sizeof(response), stdin);

                json_object_set_new(jsonObject, response, json_array());

                sections = realloc(sections, (sectionsNum + 1) * sizeof(char *));
                if (sections == NULL)
                {
                    fprintf(stderr, "Memory allocation error\n");
                    pause();
                    return 1;
                }
                sections[sectionsNum] = strdup(response);
                if (sections[sectionsNum] == NULL)
                {
                    fprintf(stderr, "Memory allocation error\n");
                    pause();
                    return 1;
                }
                sectionsNum++;

                continue;
            }

            else if (strcmp(response, "r") == 0)
            {
                // Removes a section

                while (true)
                {
                    char response[100];
                    printf("Enter which section [number] to be removed > ");
                    readLine(response, sizeof(response), stdin);
                    if (analyzeString(response) == "NUMBERS")
                    {
                        int sectionNum = atoi(response);
                        if (sectionNum > 0 && sectionNum <= sectionsNum)
                        {
                            int sectionIndex = sectionNum - 1;
                            removeKey(jsonObject, sections[sectionIndex]);
                            removeElement(sections, &sectionsNum, sectionIndex);
                            break;
                        }
                    }
                    printf("Invalid input.\n");
                }
                continue;
            }

            else if (strcmp(response, "q") == 0)
            {
                // Quits the program;
                break;
            }
            else
            {
                invalidInput = true;
                continue;
            }
        }
    }

    // [4] Stores the changes to the students-data.json file

    char *json_string = json_dumps(jsonObject, JSON_INDENT(2));

    FILE *file = fopen("students-data.json", "w");
    if (file == NULL)
    {
        fprintf(stderr, "error: could not open file\n");
        pause();
        return 1;
    }

    if (fputs(json_string, file) == EOF)
    {
        fprintf(stderr, "error: could not write to file\n");
        pause();
        return 1;
    }

    // Closes/frees files/resources
    fclose(file);
    free(json_string);
    json_decref(jsonObject);

    pause();
    return 0;
}
