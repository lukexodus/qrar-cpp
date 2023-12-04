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
    freeArrayOfStrings(filenames); // Free resources

    json_t *jsonObject = json_object(); // Object to where the data in the file is placed
    int sectionsNum = 0;                // Track the number of sections
    char **sections = NULL;             // Array of strings (section names)

    // [2] Retrieves the necessary data

    if (!jsonFileExists)
    {
        // [2A] INITIALIZES DATA

        // [2A.1] Get the sections of the college from the user

        char response[100]; // 99 characters maximum string length, the 100th character is the \0 (null character)

        printf("Input all sections of the college, for example \"BSCS 1B\", \"BSIT 4A\", etc\n");
        printf("Press Enter if done\n");

        // Continuously inputs the section names
        while (true)
        {
            printf("Enter section name: ");
            // `readLine` is a wrapper function around `fgets` that automatically
            // removes the ending newline (\n) character
            // `stdin` is the standard input stream, for inputting stuff
            readLine(response, sizeof(response), stdin);
            // or use `scanf("%s", &response);` but fgets is safer

            // Check if the entered string is empty (Enter key pressed)
            if (response[0] == '\0')
            {
                // Break the loop
                break;
            }

            // Dynamic allocation the array of section names (strings)
            sections = realloc(sections, (sectionsNum + 1) * sizeof(char *));
            if (sections == NULL) // If allocation fails
            {
                // End the program
                fprintf(stderr, "Memory allocation error\n");
                pause();
                return 1;
            }
            sections[sectionsNum] = strdup(response); // Adds the response (section name) to the array
            if (sections[sectionsNum] == NULL)        // If it fails
            {
                fprintf(stderr, "Memory allocation error\n");
                pause();
                return 1;
            }
            sectionsNum++;
        }

        // [2A.2] Writes sections to json file

        // Creates students-data.json file
        FILE *file = fopen("students-data.json", "w"); // "w" means write
        if (file == NULL)
        {
            fprintf(stderr, "error: could not open file\n");
            pause();
            return 1;
        }

        // Creates the json object
        for (int i = 0; i < sectionsNum; ++i)
        {
            // json_object_set_new sets the value of the property specified
            json_object_set_new(jsonObject, sections[i], json_array());
        }

        // Write the json to file
        // `json_dumps` converst the JSON object into string
        // `JSON_INDENT(2)` means to indent (2 spaces) the JSON
        // string so that it will be easier to read
        char *jsonStringValue = json_dumps(jsonObject, JSON_INDENT(2));
        if (fputs(jsonStringValue, file) == EOF)
        { // EOF is end-of-file i.e. an error occured
            fprintf(stderr, "error: could not write to file\n");
            pause();
            return 1;
        }

        fclose(file); // Close the file to free resources
    }
    else
    {
        // [2B] RETRIEVES PREVIOUSLY STORED DATA

        // [2B.1] Get data from students-data.json file

        // Opens students-data.json file and reads its contents
        // The `readFileStatic` reads the text from 'students-data.json' file,
        // and then puts it on the `jsonString` variable
        char jsonString[1024]; // 1024 bytes or 1 megabyte
        if (!readFileStatic("students-data.json", jsonString, sizeof(jsonString)))
        {
            fprintf(stderr, "Error reading input\n");
            pause();
            return 1;
        }

        // Convert the string to a JSON object
        json_error_t error; // Variable where the error info will be stored
        jsonObject = json_loads(jsonString, 0, &error);
        if (!jsonObject) // If it fails
        {
            fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
            pause();
            return 1;
        }

        // Gets all sections registered
        const char *key; // The property name
        json_t *value;   // JSON values should be of type `json_t`

        // Iterates the key-value pairs of the `jsonObject`
        json_object_foreach(jsonObject, key, value)
        {
            // Allocates memory for one string value (char *) for the section name.
            // `char *` means a pointer to the first element of an
            // array of characters (one string value), sizeof gets its size
            // multiplied by the numbers of sections so far
            sections = realloc(sections, (sectionsNum + 1) * sizeof(char *));
            if (sections == NULL)
            {
                fprintf(stderr, "Memory allocation error\n");
                pause();
                return 1;
            }
            // Adds the section name to the `sections` array
            // by using the `sectionsNum` as the index
            sections[sectionsNum] = strdup(key);
            if (sections[sectionsNum] == NULL)
            {
                fprintf(stderr, "Memory allocation error\n");
                pause();
                return 1;
            }
            sectionsNum++; // Increase the count of the sections
        }
    }

    // [3] Goes to edit mode

    // Variable that tracks if the inputted value is valid or not
    bool invalidInput = false;

    // Holder for the chosen section to be used [3.3] Students list loop
    char *chosenSection;

    // Main loop
    while (true)
    {
        // Displays the list of sections
        system("cls"); // Clear all the outputted text in the terminal/console

        char dashPadding[] = "------------------";
        printf("%s Sections %s\n\n", dashPadding, dashPadding);
        // If there are sections registered
        if (sectionsNum > 0)
        {
            for (int i = 0; i < sectionsNum; ++i)
            {
                printf("[%d] %s\n", i + 1, sections[i]);
            }
        }
        // Else display the following
        else
        {
            printf("No sections configured yet");
        }
        printf("\n----------------------------------------------\n");

        // Instructions
        printf("\nSelect section to edit\n\nOR\n\n[a] Add section\n[r] Remove section\n[q] (Quit) Save changes and exit program\n\n");

        // If the previous input was invalid
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

        char response[100];
        readLine(response, sizeof(response), stdin);

        // [3.2] Action

        // If the input contains numbers only
        if (analyzeString(response) == "NUMBERS")
        {
            // [3.2A] If the user wants to select a section

            // atoi ASCII to integer i.e. converts the text to integer type
            int num = atoi(response);

            // If within range
            if (num > 0 && num <= sectionsNum)
            {
                // Used for going back to the main loop from the following loop
                bool continueMainLoop = false;

                bool invalidInput2 = false;

                // [3.3] Students list loop
                while (true)
                {
                    // Gets the list of students in the specified section
                    chosenSection = sections[num - 1]; // Minus one becomes indexes start at 0

                    // `json_object_get` gets the value of a certain property
                    json_t *studentsArrayInSection = json_object_get(jsonObject, chosenSection);

                    // `json_array_size` gets the size of the array (json_array)
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

                    // Instructions
                    printf("\nSelect which student to edit\n\nOR\n\n[a] Add student\n[r] Remove student\n[b] Go back\n\n");

                    // If the previous input is invalid
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
                    char response[100];
                    readLine(response, sizeof(response), stdin);

                    if (analyzeString(response) == "NUMBERS")
                    {
                        int num = atoi(response);
                        if (num > 0 && num <= studentsInsectionsNum)
                        {
                            // Retrieves the current student object
                            json_t *studentObject = json_array_get(studentsArrayInSection, num - 1);

                            // Inputs new name
                            char newName[100];
                            printf("Enter the new name [%s] > ", json_string_value(json_object_get(studentObject, "name")));

                            readLine(newName, sizeof(newName), stdin);

                            if (strlen(newName) != 0)
                            {
                                json_t *newNameVal = json_string(newName);
                                modifyObject(studentObject, "name", newNameVal);
                                json_decref(newNameVal);
                            }

                            // Inputs new ID
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
                        // If outside of range
                        else
                        {
                            invalidInput2 = true;
                            continue;
                        }
                    }
                    else if (analyzeString(response) == "LETTERS")
                    {
                        // Checks if the reponse is "a"
                        if (strcmp(response, "a") == 0)
                        {
                            // Adds student

                            // Inputs student name
                            char *name[100], *id[10];
                            printf("\nEnter student name > ");
                            readLine(name, sizeof(name), stdin);

                            // Inputs student ID
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
                                // Inputs id of the student to be removed
                                char *id[10];
                                printf("\nEnter the ID of the student to be removed > ");
                                readLine(id, sizeof(id), stdin);

                                // If the student with the inputted ID exists
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

        // If the input contains letter only
        else if (analyzeString(response) == "LETTERS")
        {
            if (strcmp(response, "a") == 0)
            {
                // Adds section

                // Inputs section name
                char response[100];
                printf("Enter the name of the section to be added > ");
                readLine(response, sizeof(response), stdin);

                // Initializes the new key of the JSON which is the section name
                // with an empty array (to where the student objects will be stored)
                json_object_set_new(jsonObject, response, json_array());

                // Allocates memory for one string value
                sections = realloc(sections, (sectionsNum + 1) * sizeof(char *));
                if (sections == NULL)
                {
                    fprintf(stderr, "Memory allocation error\n");
                    pause();
                    return 1;
                }

                // Stores the response (section name) into the array
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
                    // Input section number to be removed
                    char response[100];
                    printf("Enter which section [number] to be removed > ");
                    readLine(response, sizeof(response), stdin);

                    if (analyzeString(response) == "NUMBERS")
                    {
                        int sectionNum = atoi(response);
                        if (sectionNum > 0 && sectionNum <= sectionsNum)
                        {
                            int sectionIndex = sectionNum - 1;
                            // Remove section from the `jsonObject`
                            printf("DEBUG P1 %d\n", sectionIndex);
                            removeKey(jsonObject, sections[sectionIndex]);
                            // Remove section from the `sections` array
                            printf("DEBUG P2\n");
                            removeElementFromArrayOfStrings(sections, &sectionsNum, sectionIndex);
                            printf("DEBUG P3\n");
                            break;
                        }
                    }
                    // If the input contains character other than numbers
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

    // `json_dumps` converts a JSON object into string
    char *json_string = json_dumps(jsonObject, JSON_INDENT(2));

    // Overwrites the students-data.json file with the new data
    FILE *file = fopen("students-data.json", "w");
    if (file == NULL)
    {
        fprintf(stderr, "error: could not open file\n");
        pause();
        return 1;
    }

    // Stores the string into the file
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