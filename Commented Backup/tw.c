// COMP2521 21T2 Assignment 1
//
// Summary:  tw.c ... Computes top N most frequent words in file F
//
// Overview: Reads number of required words (optional, defaults to 10) and 
//           filename as arguments from stdin. Assuming that the input file
//           is a Project Gutenberg book in plain text format, reads contents
//           of the book only. Each line is tokenised and normalised, then
//           single character words and stopwords are discarded and the
//           remaining words are stemmed. Resulting stems are stored in a BST
//           and once full contents has been read, the requested number of stems
//           are printed as output, ordered first by frequency then by 
//           lexicographical order.
//
// Author:   Lachlan Scott (z5207471
//
// Written:  21/6/21 - 16/7/21
//
// Usage:    ./tw [Nwords] File

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Dict.h"
#include "stemmer.h"
#include "WFreq.h"

#define TRUE 1
#define FALSE 0

#define MAXLINE 1000
#define MAXLINEWORDS 500
#define MAXWORD 100
#define NSTOPWORDS 654
#define SWLENRANGE 12

#define STARTSTRING "*** START OF"
#define STARTSTRINGLEN (int)(sizeof(STARTSTRING) - 1)
#define ENDSTRING "*** END OF"
#define ENDSTRINGLEN (int)(sizeof(ENDSTRING) - 1)

#define isWordChar(c) (isalnum(c) || (c) == '\'' || (c) == '-')

typedef char word[MAXWORD];

typedef struct wordline *WordLine;

struct wordline {
    word* words;
    int size;
};

WordLine NewWordLine(void);

void FreeWordLine(WordLine w);

void WLAppendWord(WordLine l, char* w, int length);

WordLine *getStopWords();

int isStopWord(char *line, int start, int end, WordLine *stopWords);

void normalise(char* line, int start, int end);

WordLine processLine(char* line, int length, WordLine *stopWords);

int main(int argc, char *argv[]) {
    int   nWords;    // Number of top frequency words to show
    char *fileName;  // Name of file containing book text
    
    // Process command-line args
    switch (argc) {
        case 2: // No word count specified, default to 10
            nWords = 10;
            fileName = argv[1];
            break;
        case 3: // Word count and file name specified
            nWords = atoi(argv[1]);
            if (nWords < 10) nWords = 10;
            fileName = argv[2];
            break;
        default: // Wrong number of arguments, raise error
            fprintf(stderr,"Usage: %s [Nwords] File\n", argv[0]);
            exit(EXIT_FAILURE);
    }

    FILE *fp;
    fp = fopen(fileName, "r");

    // Handle case where filename in command argument can't be opened
    if (fp == NULL) {
        fprintf(stderr, "Can't open %s\n", fileName);
        exit(EXIT_FAILURE);
    }

    WordLine *stopWords = getStopWords();

    Dict d = DictNew(); // Dict to store words

    char line[MAXLINE+1]; // Stores each line of the given file for processing
    int foundStart = FALSE; // Flag, track whether start line was found
    int foundEnd = FALSE; // Flag, track whether end line was found

    // Read each line of the file, process lines between PG start and end lines
    while (fgets(line, MAXLINE+1, fp) != NULL) {
        // Replace newline with end char to simplify getting line length
        line[strcspn(line, "\n")] = '\0';
        int len = strlen(line);

        // Only process text between the PG start and end lines
        if (foundStart == TRUE) {
            if (len >= ENDSTRINGLEN && 
                    strncmp(line, ENDSTRING, ENDSTRINGLEN) == 0) {
                foundEnd = TRUE; // PG end line has been found, end text processing
                break;
            }

            // Process and extract words from line
            WordLine lineWords = processLine(line, len, stopWords);

            // Insert each processed word into the dict
            for (int i = 0; i < lineWords->size; i++) {
                DictInsert(d, lineWords->words[i]); 
            }

            FreeWordLine(lineWords);
        } else if (len >= STARTSTRINGLEN && 
                strncmp(line, STARTSTRING, STARTSTRINGLEN) == 0) {
            foundStart = TRUE; // PG start line has been found, begin text processing
        }
    }
    fclose(fp);

    // Free stopword array of wordlines
    for (int i = 0; i < SWLENRANGE; i++) { 
        FreeWordLine(stopWords[i]);
    }
    free(stopWords);

    // Handle case where start and/or end string were not found
    if (foundStart == FALSE || foundEnd == FALSE) { 
        DictFree(d);
        fprintf(stderr, "Not a Project Gutenberg book\n");
        exit(EXIT_FAILURE);
    } else { 
        // Found both start and end strings, print requested number of WFreqs

        WFreq *wfs = malloc(sizeof(WFreq)*nWords);

        // Retrieve top n words by freq., in lexicographical order
        int nWordsFound = DictFindTopN(d, wfs, nWords);

        // Print the returned WFreqs in required format
        for (int i = 0; i < nWordsFound; i++) {
            printf("%d %s\n", wfs[i].freq, wfs[i].word);
        }
        free(wfs);
        DictFree(d);
    }
}

// Allocates memory for and initialises a wordline struct
// Returns a WordLine pointer to a wordline struct
// Raises an error if memory couldn't be allocated
WordLine NewWordLine(void) {
    WordLine w = (WordLine)malloc(sizeof(*w));
    if (w == NULL) {
        fprintf(stderr, "couldn't allocate WordLine");
        exit(EXIT_FAILURE);
    }

    w->words = calloc(MAXWORD, sizeof(char)*MAXLINEWORDS);
    if (w->words == NULL) {
        fprintf(stderr, "couldn't allocate WordLine words");
        exit(EXIT_FAILURE);
    }

    w->size = 0;
    return w;
}

// Frees the memory allocated to a wordline struct
void FreeWordLine(WordLine w) {
    if (w != NULL) {
        if (w->words != NULL) {
            free(w->words);
        }
        free(w);
    }
}

// Appends a substring of a given length to the words array of a wordline
// Increments wordline size to reflect newly added word
void WLAppendWord(WordLine l, char* w, int length) {
    memcpy(l->words[l->size], w, length); // Copy word to words array
    l->words[l->size][length] = 0; // Set end character of word to 0
    l->size++;
}

// Returns an array of wordline structs containing all stopwords longer than
// one character. Array index "i" stores words of length "i+2".
WordLine *getStopWords() {
    FILE *sw;

    // Initialise the stopwords array of wordline structs
    WordLine *stopWords = (WordLine *)malloc(sizeof(WordLine) * SWLENRANGE);
    for (int i = 0; i < SWLENRANGE; i++) {
        stopWords[i] = NewWordLine();
    }

    sw = fopen("stopwords", "r");

    if (sw == NULL) {
        fprintf(stderr, "Can't open stopwords\n");
        exit(EXIT_FAILURE);
    }

    word stopWord;

    // Collect all necessary stopwords from stopwords file
    while (fgets(stopWord, MAXWORD+1, sw) != NULL) {
        // Get length of stopword
        int l = 0;
        while (stopWord[l] != '\n') {
            l++;
        }

        // Store stopwords longer than one char.
        // Single char. words are filtered out separately
        if (l > 1) {
            WLAppendWord(stopWords[l-2], stopWord, l);
        }
    }
    fclose(sw);
    return stopWords;
}

// Checks whether a given section of a line is a stopword
// Returns TRUE (1) or FALSE (0)
int isStopWord(char *line, int start, int end, WordLine *stopWords) {
    int length = end - start;

    // If word is longer than the longest stopword, match is impossible
    if (length > SWLENRANGE + 1) {
        return FALSE;
    }

    word checkWord;
    memcpy(checkWord, line + start, length); // Copy word from line into checkWord array
    checkWord[length] = 0; // Add terminating character to end of word string

    // Iterate through stopwords of the same length
    for (int i = 0; i < stopWords[length-2]->size; i++) {
        int cmp = strcmp(checkWord, stopWords[length-2]->words[i]);
        if (cmp < 0) { // Word comes before current stopword in lex. order, no match is possible
            return FALSE;
        }

        if (cmp == 0) { // Exact match, word is in list of stopwords
            return TRUE;
        }
    }
    return FALSE; // Word was not in list of stopwords
}

// Normalises a given substring of a char array
void normalise(char* line, int start, int end) {
    for (int i = start; i < end; i++) {
        line[i] = tolower(line[i]);
    }
}

// Returns a wordline struct containing the stems of the set of normalised, 
// tokenised words in a char array that are longer than one character and 
// not stopwords.
WordLine processLine(char line[MAXLINE], int length, WordLine *stopWords) {
    WordLine lineWords = NewWordLine();

    // No characters in line, nothing to process
    if (length == 0) {
        return lineWords;
    }

    int start = 0;
    int end = 0;
    
    // Scan and process entire line
    while (start < length) {
        // Find next valid word character, to be start of next word
        while (start < length && !isWordChar(line[start])) {
            start++;
        }

        end = start+1;

        // Find first non-word character to be end of next word
        while (end < length && isWordChar(line[end])) {
            end++;
        }

        // Process word and append it to lineWords
        int wordLength = end - start;
        if (wordLength > 1) { // Skip single-character words
            normalise(line, start, end);

            if (isStopWord(line, start, end, stopWords) == FALSE) { // Skip stopwords
                WLAppendWord(lineWords, line+start, wordLength);
                stem(lineWords->words[lineWords->size-1], 0, wordLength-1);
            }
        }
        end++;
        start = end;
        end++;
    }
    return lineWords;
}
