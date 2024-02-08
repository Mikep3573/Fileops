// jdh CS3010 Fall 2023
//
// od commands are:
// od -Ad -t u8 -v *.dat
// od -Ad -c -v *.dat

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "fileops.mpiscion.h"

// header will have 26 longs + 26 longs (one for each letter)

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

/**
 * Simply checks if a return code is 0, returns True if so, false otherwise
 * Parameters:
 * Return:
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
 * Parameters:
 * Return:
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
 * Parameters:
 * Return:
*/
bool doSeek(FILE *fp, long int offset, int whence) {
    int rc = fseek(fp, offset, whence);
    if(!checkReturnVal("fseek()", rc)) {
        return false;
    }
    return true;
}

/** 
 * TODO: Write this
*/
bool doRead(void *ptr, size_t size, size_t nmemb, FILE *fp) {
    size_t read = fread(ptr, size, nmemb, fp);
    if(read != nmemb) {
        // Normally this would include end of file, but with a strict use on how we use fread we
        // can prevent this and thus read != nmemb only ever occurs with an error
        printf("Failed reading from file");
        return false;
    }
    return true;
}

/** 
 * TODO: Write this
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
 * TODO: Write this
*/
/**
 * TODO: Write this
*/
int insertWord(FILE *fp, char *word) {
    /** TODO: Do error detection for reads and writes */
    // Check word first
    if(checkWord(word) != 0) {
        return 1;
    }

    // Convert word to lowercase
    char *convertedWord = malloc(sizeof(MAXWORDLEN + 1));
    int result = convertToLower(word, convertedWord);

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
            return 1;
        }
        if(!doWrite(wRecord, sizeof(WordRecord), 1, fp)) {
            return 4;
        }

        // No errors
        free(fHeader);
        free(wRecord);
        if(!doSeek(fp, 0, SEEK_SET)) {
            return 1;
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
            return 1;
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
                return 1;
            }
            if(!doWrite(fHeader, sizeof(FileHeader), 1, fp)) {
                return 4;
            }

            // Go to end of file
            if(!doSeek(fp, 0, SEEK_END)) {
                return 1;
            }

            // Write the word
            if(!doWrite(wRecord, sizeof(WordRecord), 1, fp)) {
                return 4;
            }

            // Reset the pointer
            if(!doSeek(fp, 0, SEEK_SET)) {
                return 1;
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
                    return 1;
                }
                if(!doRead(&newPos, sizeof(long), 1, fp)) {
                    return 3;
                }
            }

            // Write the new header
            if(!doSeek(fp, 0, SEEK_SET)) {
                return 1;
            }
            if(!doWrite(fHeader, sizeof(FileHeader), 1, fp)) {
                return 4;
            }

            // Variables for updating the old word
            WordRecord *uRecord = malloc(sizeof(WordRecord));
            long fileSize = sizeOfFile(fp);

            // Read the word from the file
            if(!doSeek(fp, lastPos, SEEK_SET)) {
                return 1;
            }
            if(!doRead(&uRecord->word, MAXWORDLEN + 1, 1, fp)) {
                return 3;
            }
            if(!doSeek(fp, lastPos + MAXWORDLEN + 1, SEEK_SET)) {
                return 1;
            }
            if(!doRead(&uRecord->nextpos, sizeof(long), 1, fp)) {
                return 3;
            }

            // Update old word
            uRecord->nextpos = fileSize;
            if(!doSeek(fp, lastPos, SEEK_SET)) {
                return 1;
            }
            if(!doWrite(uRecord, sizeof(WordRecord), 1, fp)) {
                return 4;
            }
            
            // Finish new word setup
            wRecord->nextpos = newPos;

            // Go to the end of the file
            if(!doSeek(fp, 0, SEEK_END)) {
                return 1;
            }

            // Write the word
            if(!doWrite(wRecord, sizeof(WordRecord), 1, fp)) {
                return 4;
            }

            // Reset the pointer
            if(!doSeek(fp, 0, SEEK_SET)) {
                return 1;
            }

            // Free up uRecord memory
            free(uRecord);
        }

        // Free up memory and exit
        free(fHeader);
        free(wRecord);
        return 0;
    }
}

/**
 * TODO: Write this
*/
int countWords(FILE *fp, char letter, int *count) {
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
 * TODO: Write this
*/
char *getWord(FILE *fp, char letter, int index) {
    // Check if file empty
    if(sizeOfFile(fp) == 0) {
        return NULL;
    }

    // Convert letter to lowercase
    char convertedChar;
    convertedChar = tolower(letter);

    // Get char index (return 1 if not alpha)
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
    return wRecord->word;
}

//---------------------------------------------------------------
// - write "Nineveh" to the file (as "nineveh")
// - write "kerfuffle" to the file
// - attempt to write "No Can Do" to the file; this should fail
// - write "nincompoop" to the file
// - then check

int writeTestOne(char *filename, char **existingWords[], int existingCounts[], int newfileFlag) {
  char buf[1+MAXWORDLEN];
  int rc, i, j, numWords, fileExists;
  FILE *fp;

  char *empty[] = {""};
  char *ns[] = {"Nineveh", "No Can Do", "nincompoop", ""};
  char *ks[] = {"kerfuffle", ""};
  char **newWords[] = { empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, ks, empty, empty, ns, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty};
  int newCounts[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  // if newFileFlag is set, the rewrite (open and delete) the file
  // if newFIleFlag is not set, then try to open the file in append mode

  fileExists = 0;

  if ( ! newfileFlag ) {
    fp = (FILE *) fopen(filename, "r+");
    if (fp != NULL)
      fileExists = 1;
  }

  if ( newfileFlag || ! fileExists ) {
    fp = (FILE *) fopen(filename, "w+");
    if (fp == NULL) {
      printf("cannot open file '%s'\n", filename);
      return 8;
    }
  }

  // insert n and k, alternating
  int flag, nidx, kidx;
  flag = 0;
  nidx = 0;
  kidx = 0;
  numWords = sizeof(ns) / sizeof(char *);
  numWords = numWords + sizeof(ks) / sizeof(char *);
  numWords = numWords - 2; // the "" at the end of each list
  for (i=0; i<numWords; ++i) {
    if ( ! flag ) {
      flag = 1;
      if (nidx < (sizeof(ns) / sizeof(char *)) - 1 ) {
        strcpy(buf, ns[nidx]);
        ++nidx;
      } else {
        strcpy(buf, ks[kidx]);
        ++kidx;
      }
    } else {
      flag = 0;
      if (kidx < (sizeof(ks) / sizeof(char *)) - 1 ) {
        strcpy(buf, ks[kidx]);
        ++kidx;
      } else {
        strcpy(buf, ns[nidx]);
        ++nidx;
      }
    }

    printf("insert '%s'\n", buf);
    rc = insertWord(fp, buf);
    printf("rc = %d\n", rc);
  } // for

  // add in the existing
  for (i=0; i<NUMLETTERS; ++i) {
    existingCounts[i] = newCounts[i] + existingCounts[i];
    int numWords_1 = 0;
    while (strlen(newWords[i][numWords_1]) > 0)
      numWords_1 = numWords_1 + 1;
    // printf("*** new %c has %d words\n", (i+'a'), numWords_1);

    int numWords_2 = 0;
    while (strlen(existingWords[i][numWords_2]) > 0)
      numWords_2 = numWords_2 + 1;
    // printf("*** existing %c has %d words\n", (i+'a'), numWords_2);
    int total = numWords_1 + numWords_2;
    char **w = (char **) malloc((total + 1) * sizeof(char *));
    for (j=0; j<total; ++j) {
      w[j] = malloc(1 + MAXWORDLEN);
      if (j < numWords_1)
        strcpy(w[j], newWords[i][j]);
      else
        strcpy(w[j], existingWords[i][j-numWords_1]);
    }
    w[total] = malloc(4); // big enough to hold terminating ""
    strcpy(w[total], "");
    existingWords[i] = w;
  }

  fclose(fp);
  return 0;
} // writeTestOne()

//---------------------------------------------------

int checkFile(char *filename, char **existingWords[], int existingCounts[]) {
  int i, j, rc, numWords;
  char convertedWord[1+MAXWORDLEN];
  FILE *fp;
  char *word;

  // open the file for read
  fp = (FILE *) fopen(filename, "r");
  if (fp == NULL) {
    printf("ERROR in checkFile(): cannot open '%s' for read\n", filename);
    return 8;
  }

  for (char ch='a'; ch<='z'; ++ch) {
    int charIdx = ch - 'a';
    int numGood = 0;
    j = 0;
    while (j < existingCounts[charIdx]) {
      if (checkWord(existingWords[charIdx][j]) == 0)
        ++numGood;
      j = j + 1;
    }
    rc = countWords(fp, ch, &numWords);
    if (rc != 0) {
      printf("ERROR %d from countWords() for '%c' in checkFile()\n", rc, ch);
      fclose(fp);
      return 8;
    }

    if (numWords != numGood) {
      printf("ERROR: expect %d from countWords() for '%c'; got %d in checkFile()\n", numGood, ch, numWords);
      fclose(fp);
      return 8;
    }

    int wordIdx = 0;
    for (j=0; j<existingCounts[charIdx]; ++j) {
      if (checkWord(existingWords[charIdx][j]) == 0) {
        convertToLower(existingWords[charIdx][j], convertedWord);
        word = getWord(fp, ch, wordIdx);
        if (word != NULL) {
          printf("words[%d] = '%s'\n", wordIdx, word);
          if ( strcmp(word, convertedWord) ) {
            printf("ERROR: expect '%s' from getWord(); got '%s' in checkFile()\n", convertedWord, word);
            fclose(fp);
            return 8;
          }
          free(word);
        } else {
          printf("ERROR: expect '%s' from getWord(); got NULL in checkFile()\n", convertedWord);
          fclose(fp);
          return 8;
        }
        ++wordIdx;
      } else {
        ; // skip this word: checkWord() returned zero
      }
    } // retrieved word is not NULL
  } // for

  fclose(fp);

  printf("PASS checkFile()\n");
  return 0;
} // checkFile()

//---------------------------------------------------------------

int listAll(FILE *fp) {
  char *word;
  char c;
  int i, rc, numWords;

  for (c='a'; c<='z'; ++c) {
    rc = countWords(fp, c, &numWords);
    if (rc != 0) {
      printf("ERROR %d from countWords() for '%c'\n", rc, c);
      return 8;
    }

    if (numWords > 0) {
      printf("words starting with %c (%d words):\n", c, numWords);
      for (i=0; i<numWords; ++i) {
        word = getWord(fp, c, i);
        if (word != NULL) {
          printf("words[%d] = '%s'\n", i, word);
          free(word);
        } else {
          printf("error: unexpected NULL\n");
        }
      }
    }
  }
  return 0;
} // listAll()

//---------------------------------------------------------------

int main() {
  char *filename = "testfile.dat";
  int i, rc, newfileFlag;
  FILE *fp;
  char buf[1 + MAXWORDLEN];

  char *empty[] = {""};
  char **existingWords[] = { empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty};
  int existingCounts[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  newfileFlag = 1;
  rc = writeTestOne(filename, existingWords, existingCounts, newfileFlag);
  if (rc != 0) {
    printf("writeTestOne() failed\n");
    return 8;
  }

  rc = checkFile(filename, existingWords, existingCounts);
  if (rc != 0) {
    printf("checkFile() failed\n");
    return 0;
  }

  // open the file for append
  fp = (FILE *) fopen(filename, "r+");
  if (fp == NULL) {
    printf("UNEXPECTED ERROR: cannot open '%s' for append\n", filename);
    return 8;
  }

  printf("\n");
  printf("---------------------------------------------------\n");
  printf("here are the words in the file after writeTestOne()\n");
  listAll(fp);
  printf("---------------------------------------------------\n");
  printf("\n");

  char *as[] = {"acacia", "avatar", ""};

  strcpy(buf, as[0]);
  rc = insertWord(fp, buf);
  if (rc != 0) {
    printf("UNEXPECTED ERROR: cannot write '%s' to file\n", buf);
    fclose(fp);
    return 0;
  }

  strcpy(buf, as[1]);
  rc = insertWord(fp, buf);
  if (rc != 0) {
    printf("UNEXPECTED ERROR: cannot write '%s' to file\n", buf);
    fclose(fp);
    return 0;
  }

  printf("\n");
  printf("-----------------------------------------------------------\n");
  printf("here are the words in the file after writing two more words\n");
  listAll(fp);
  printf("-----------------------------------------------------------\n");
  printf("\n");

  fclose(fp);

  existingWords[0] = as;
  existingCounts[0] = 2;

  rc = checkFile(filename, existingWords, existingCounts);
  if (rc != 0) {
    printf("checkFile() failed\n");
    return 0;
  }

  newfileFlag = 0;
  rc = writeTestOne(filename, existingWords, existingCounts, newfileFlag);
  if (rc != 0) {
    printf("writeTestOne() failed\n");
    return 8;
  }

  rc = checkFile(filename, existingWords, existingCounts);
  if (rc != 0) {
    printf("checkFile() failed\n");
    return 0;
  }
  return 0;
} // main()
