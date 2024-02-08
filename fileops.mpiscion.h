/**
 * This file is the .h file for Programming Assignment #6 File Operations
 * This was written by Michael Piscione
*/

// Dependencies
#include <stdio.h>
#include <stdbool.h>

// Define constants
#define MAXWORDLEN 31
#define NUMLETTERS 26

// Structs
typedef struct {
    long counts[NUMLETTERS];
    long startPositions[NUMLETTERS];
} FileHeader;

typedef struct {
    char word[1 + MAXWORDLEN];
    long nextpos;
} WordRecord;

// Functions
int checkWord(char *word);
int convertToLower(char *word, char *convertedWord);
bool checkReturnVal(char *funcToPrint, int rc);
long sizeOfFile(FILE *fp);
int getCharIndex(char character);
bool doSeek(FILE *fp, long int offset, int whence);
bool doRead(void *ptr, size_t size, size_t nmemb, FILE *fp);
bool doWrite(const void *ptr, size_t size, size_t nmemb, FILE *fp);
int insertWord(FILE *fp, char *word);
int countWords(FILE *fp, char letter, int *count);
char *getWord(FILE *fp, char letter, int index);

