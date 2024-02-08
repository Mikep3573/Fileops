/**
 * This file is the .c file for Programming Assignment #6 File Operations
 * This was written by Michael Piscione
*/

// Dependencies
#include "fileops.mpiscion.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Prof. Hibbeler's Utility Functions
//--------------------------------------------------------
// return 1 if any chararacters are non-alphabetic;
// otherwise return 0

int checkWord(char *word) {
  int i, len;

  len = strlen(word);
  for (i=0; i<len; ++i) {
    if ( ! isalpha(word[i]) )
      return 1;
  }

  return 0;
}

//--------------------------------------------------------
// convert ach character to its lower-case equivalent;
// leaves characters that are already lower case unchanged;
// returns zero always;
// assumes that convertedWord has sufficient space to hold
// the result

int convertToLower(char *word, char *convertedWord) {
  int i, len;

  strcpy(convertedWord, word);

  len = strlen(word);
  for (i=0; i<len; ++i)
    convertedWord[i] = tolower(word[i]);

  return 0;
}

// My Work
/**
 * Simply checks if a return code is 0, returns True if so, false otherwise. Gives a custom
 * print statement dependent on parameter funcToPrint.
 * Parameters: char *funcToPrint (is put into a custom error statement to be printed), rc (the return code to check)
 * Return: bool, true if rc zero, false otherwise
*/
bool checkReturnVal(char *funcToPrint, int rc) {
    if(rc != 0) {
        printf("%s failed \n", funcToPrint);
        return false;
    }
    return true;
}

/**
 * with a file pointer as input, returns the size of the file
 * Parameters: FILE *fp, the file pointer
 * Return: long, the size of the file
*/
long sizeOfFile(FILE *fp) {
    int rc;

    // Check if seek failed, if not, return filesize
    rc = fseek(fp, 0, SEEK_END);
    if(!checkReturnVal("fseek()", rc)) {
        return rc;
    }
    
    // Get the size of the file (end-of-file position) and reset pointer
    long size = ftell(fp);
    rc = fseek(fp, 0, SEEK_SET);
    if(!checkReturnVal("fseek()", rc)) {
        return rc;
    }

    return size;
}

/**
 * Returns an array index of an alphabetical character
 * Parameters: char character (the character to get the index of)
 * Return: int, the index of the character
*/
int getCharIndex(char character) {
    char letters[NUMLETTERS] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

    // Get which index in the array the letter appears at (array is in alpha order)
    for(int i = 0; i < NUMLETTERS; i++) {
        if(letters[i] == character) {
            return i;
        }
    }

    // Return 1 for failed result
    printf("Invalid char in getCharIndex\n");
    return -1;
}

/** 
 * doSeek takes all of fseek's input parameters as input (FILE *, long int, int).
 * It performs the seek and necessary error catching. If an error occurs, it returns false,
 * true otherwise.
 * Parameters: fseek's input parameters (file pointer, offset, starting place constant)
 * Return: bool, true if fseek was performed successfully, false otherwise
*/
bool doSeek(FILE *fp, long int offset, int whence) {
    int rc = fseek(fp, offset, whence);
    if(!checkReturnVal("fseek()", rc)) {
        return false;
    }
    return true;
}

/** 
 * doRead takes all of fread's input parameters as input (void *, size_t, size_t, FILE *) and
 * performs fread on them. Does any error catching. Returns true if no errors, false otherwise.
 * Parameters: fread's input parameters (ptr to store read data, size of data, how many to read, file pointer)
 * Return: bool, true if no errors caught, false otherwise
*/
bool doRead(void *ptr, size_t size, size_t nmemb, FILE *fp) {
    size_t read = fread(ptr, size, nmemb, fp);
    if(read != nmemb) {
        // Normally this would include end-of-file, but with a strict use on how we use fread we
        // can prevent this and thus read != nmemb only ever occurs with an error
        printf("Failed reading from file");
        return false;
    }
    return true;
}

/** 
 * doWrite takes all of fwrite's input parameters as input (const void *, size_t, size_t, FILE *) and
 * performs fwrite on them. Does any error catching. Returns true if no errors, false otherwise.
 * Parameters: fwrite's input parameters (ptr to store to-write data, size of data, how many to write, file pointer)
 * Return: bool, true if no errors caught, false otherwise
*/
bool doWrite(const void *ptr, size_t size, size_t nmemb, FILE *fp) {
    size_t written = fwrite(ptr, size, nmemb, fp);
    if(written != nmemb) {
        printf("Failed writing to file");
        return false;
    }
    return true;
}

/**
 * insertWord takes a file pointer and a string as input. Upon string validation, it writes the string to
 * the end of the file. Updating the header and previous words as necessary. Note that the header keeps track
 * of the number of words for each possible starting character (alphabet), and the position of the first word
 * written under that character. Each word that is written also has the location of the next word in the list 
 * (the next word starting with the same character) written after it.
 * Parameters: FILE *fp, the file pointer, and char *word, the word to write.
 * Return: int, a return code. 1 for word invalid, 2 for fseek error, 3 for fread error, and 4 for fwrite error.
 * 0 for success.
*/
int insertWord(FILE *fp, char *word) {
    // Check word first
    if(checkWord(word) != 0) {
        return 1;
    }

    // Convert word to lowercase
    char *convertedWord = malloc(MAXWORDLEN + 1);
    convertToLower(word, convertedWord);

    // Setup information
    long nextPos = 0;
    WordRecord *wRecord = malloc(sizeof(WordRecord));
    int charIndex = getCharIndex(convertedWord[0]);
    if(charIndex == -1) {  // Exit if invalid char (shouldn't happen thanks to checkWord)
        return 1;
    }

    // Setup the word record
    strcpy(wRecord->word, convertedWord);

    if (sizeOfFile(fp) == 0) {  // If there is nothing in the file, write the header and the word
        FileHeader *fHeader = malloc(sizeof(FileHeader));

        // Setup the header using the first character of the word
        fHeader->counts[charIndex] = 1;
        fHeader->startPositions[charIndex] = sizeof(FileHeader);

        // Finish word setup
        wRecord->nextpos = nextPos;

        // Write to file
        if(!doWrite(fHeader, sizeof(FileHeader), 1, fp)) {
            return 4;
        }
        if(!doSeek(fp, sizeof(FileHeader), SEEK_SET)) {
            return 2;
        }
        if(!doWrite(wRecord, sizeof(WordRecord), 1, fp)) {
            return 4;
        }

        // No errors
        free(fHeader);
        free(wRecord);
        free(convertedWord);
        if(!doSeek(fp, 0, SEEK_SET)) {
            return 2;
        }
        return 0;
    }
    else {
        FileHeader *fHeader = malloc(sizeof(FileHeader));

        // Read the header
        for(int i = 0; i < NUMLETTERS; i++) {
            if(!doRead(&fHeader->counts[i], sizeof(long), 1, fp)) {
                return 3;
            }
        }
        if(!doSeek(fp, sizeof(fHeader->counts), SEEK_SET)) {
            return 2;
        }
        for(int i = 0; i < NUMLETTERS; i++) {
            if(!doRead(&fHeader->startPositions[i], sizeof(long), 1, fp)) {
                return 3;
            }
        }

        // Update header counters
        fHeader->counts[charIndex] += 1;
        if(fHeader->counts[charIndex] == 1) {  // First of a kind
            // Finish header and word setup
            fHeader->startPositions[charIndex] = sizeOfFile(fp);
            wRecord->nextpos = nextPos;

            // Write the new header
            if(!doSeek(fp, 0, SEEK_SET)) {
                return 2;
            }
            if(!doWrite(fHeader, sizeof(FileHeader), 1, fp)) {
                return 4;
            }

            // Go to end of file
            if(!doSeek(fp, 0, SEEK_END)) {
                return 2;
            }

            // Write the word
            if(!doWrite(wRecord, sizeof(WordRecord), 1, fp)) {
                return 4;
            }

            // Reset the pointer
            if(!doSeek(fp, 0, SEEK_SET)) {
                return 2;
            }
        }
        else {  // Or last in the list
            // Go to position of last word in first letter category
            long newPos = fHeader->startPositions[charIndex];
            long lastPos = 0;
            while(newPos != 0) {
                // Since we don't know we're done till we reach zero, we should also keep track of the previous
                // position (the position of the word we need to update)
                lastPos = newPos;
                if(!doSeek(fp, newPos + MAXWORDLEN + 1, SEEK_SET)) {
                    return 2;
                }
                if(!doRead(&newPos, sizeof(long), 1, fp)) {
                    return 3;
                }
            }

            // Write the new header
            if(!doSeek(fp, 0, SEEK_SET)) {
                return 2;
            }
            if(!doWrite(fHeader, sizeof(FileHeader), 1, fp)) {
                return 4;
            }

            // Variables for updating the old word
            WordRecord *uRecord = malloc(sizeof(WordRecord));
            long fileSize = sizeOfFile(fp);

            // Read the word from the file
            if(!doSeek(fp, lastPos, SEEK_SET)) {
                return 2;
            }
            if(!doRead(&uRecord->word, MAXWORDLEN + 1, 1, fp)) {
                return 3;
            }
            if(!doSeek(fp, lastPos + MAXWORDLEN + 1, SEEK_SET)) {
                return 2;
            }
            if(!doRead(&uRecord->nextpos, sizeof(long), 1, fp)) {
                return 3;
            }

            // Update old word
            uRecord->nextpos = fileSize;
            if(!doSeek(fp, lastPos, SEEK_SET)) {
                return 2;
            }
            if(!doWrite(uRecord, sizeof(WordRecord), 1, fp)) {
                return 4;
            }
            
            // Finish new word setup
            wRecord->nextpos = newPos;

            // Go to the end of the file
            if(!doSeek(fp, 0, SEEK_END)) {
                return 2;
            }

            // Write the word
            if(!doWrite(wRecord, sizeof(WordRecord), 1, fp)) {
                return 4;
            }

            // Reset the pointer
            if(!doSeek(fp, 0, SEEK_SET)) {
                return 2;
            }

            // Free up uRecord memory
            free(uRecord);
        }

        // Free up memory and exit
        free(fHeader);
        free(convertedWord);
        free(wRecord);
        return 0;
    }
}

/**
 * countWords takes a file pointer, a letter, and a place to store a count of the number of strings in the file
 * that start with said given letter. Simply reads the header and returns the count of a given starting letter through
 * int *count.
 * Parameters: FILE *fp, the file pointer, char letter, the starting letter, and int *count, a pointer to an integer variable
 * meant to hold the count of the strings
 * Return: int, return code. 1 for file or letter error, 2 for fseek error, and 3 for fread error.
*/
int countWords(FILE *fp, char letter, int *count) {
    // Check for empty file
    if(sizeOfFile(fp) == 0) {
        printf("Empty File\n");
        return 1;
    }

    // Convert letter to lowercase
    char convertedChar;
    convertedChar = tolower(letter);

    // Get char index (return 1 if not alpha)
    int charIndex = getCharIndex(convertedChar);
    if(charIndex == -1) {
        return 1;
    }

    // Seek to top of file (return error code if failed)
    if(!doSeek(fp, 0, SEEK_SET)) {
        return 2;
    }

    // Read header
    FileHeader *fHeader = malloc(sizeof(FileHeader));
    for(int i = 0; i < NUMLETTERS; i++) {
        if(!doRead(&fHeader->counts[i], sizeof(long), 1, fp)) {
            return 3;
        }
    }

    // Reset pointer
    if(!doSeek(fp, 0, SEEK_SET)) {
        return 2;
    }

    // Return num of words starting with that character through count
    *count = fHeader->counts[charIndex];
    free(fHeader);
    return 0;
}

/**
 * getWord takes a file pointer, a letter, and an index of the specific word in the file we're looking for.
 * getWord reads the chain of strings that start with the given letter and upon the index'th string found,
 * returns that string (NOTE index goes from 0 -> numwords-1).
 * Parameters: FILE *fp, the file pointer, char letter, the starting letter, and int index, the index of 
 * the word we're looking for
 * Return char *, the word found at that index in the file. Returns NULL if error occurs.
*/
char *getWord(FILE *fp, char letter, int index) {
    // Check if file empty
    if(sizeOfFile(fp) == 0) {
        printf("Empty File\n");
        return NULL;
    }

    // Convert letter to lowercase
    char convertedChar;
    convertedChar = tolower(letter); 

    // Get char index (return NULL if not alpha)
    int charIndex = getCharIndex(convertedChar);
    if(charIndex == -1) {
        return NULL;
    }

    // Read header
    FileHeader *fHeader = malloc(sizeof(FileHeader));
    for(int i = 0; i < NUMLETTERS; i++) {
        if(!doRead(&fHeader->counts[i], sizeof(long), 1, fp)) {
            return NULL;
        }
    }
    if(!doSeek(fp, sizeof(fHeader->counts), SEEK_SET)) {
        return NULL;
    }
    for(int i = 0; i < NUMLETTERS; i++) {
        if(!doRead(&fHeader->startPositions[i], sizeof(long), 1, fp)) {
            return NULL;
        }
    }

    // Check if index out-of-bounds
    if(index >= fHeader->counts[charIndex] || index < 0) {
        return NULL;
    }

    // Look at character start position
    int loc = fHeader->startPositions[charIndex];
    int count = -1;
    WordRecord *wRecord = malloc(sizeof(WordRecord));

    while(count != index) {
        // Seek to loc and read word record
        if(!doSeek(fp, loc, SEEK_SET)) {
            return NULL;
        }
        if(!doRead(&wRecord->word, MAXWORDLEN + 1, 1, fp)) {
            return NULL;
        }
        if(!doSeek(fp, loc + MAXWORDLEN + 1, SEEK_SET)) {
            return NULL;
        }
        if(!doRead(&wRecord->nextpos, sizeof(long), 1, fp)) {
            return NULL;
        }

        // Get the nextpos
        loc = wRecord->nextpos;

        count++;
    }

    // Return the word
    free(fHeader);
    return wRecord->word;
}

int main() {
    FILE *fp;
    char filename[MAXWORDLEN] = "words.dat";

    // Check if the file exists and create if need-be
    fp = (FILE *) fopen(filename, "r+");
    if(fp == NULL) {
        fp = (FILE *) fopen(filename, "w+");
        if(fp == NULL) {
            printf("Cannot open file with name: '%s' \n", filename);
            return 8;  // Return 8 on a failed creation
        }
    }
    else {
        return 1;
    }

    // Insert Nineveh, KerFuffle, AvalANCHe, and kranguS
    insertWord(fp, "Nineveh");
    insertWord(fp, "KerFuffle");
    insertWord(fp, "AvalANCHe");
    insertWord(fp, "kranguS");
    
    // Count words starting with A (should be 1)
    int count = 0;
    countWords(fp, 'A', &count);
    printf("Count: %d\n", count);

    // Insert zebrA
    insertWord(fp, "zebrA");

    // Count words starting with K (should be 2)
    countWords(fp, 'K', &count);
    printf("Count: %d\n", count);

    // Print size of file (header is 416 bytes and each word structure is 40)
    // By now we have 416 + (5)40 = 616 bytes
    printf("Size of file %lu\n", sizeOfFile(fp));

    // Find first and second word that starts with k (kerfuffle and krangus)
    char *word = getWord(fp, 'K', 0);
    printf("Word Found: %s\n", word);
    word = getWord(fp, 'K', 1);
    printf("Word Found: %s\n", getWord(fp, 'K', 1));
    free(word);  // NOTE: Should free the memory associated with word after use

    // Close handler when done
    fclose(fp);

    return 0;
}
