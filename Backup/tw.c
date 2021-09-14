// COMP2521 21T2 Assignment 1
// tw.c ... compute top N most frequent words in file F
// Usage: ./tw [Nwords] File

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
#define MAXWORD 100
#define NSTOPWORDS 654

#define STARTSTRING "*** START OF THIS PROJECT GUTENBERG EBOOK"
#define STARTSTRINGLEN (int)(sizeof(STARTSTRING) - 1)
#define ENDSTRING "*** END OF THIS PROJECT GUTENBERG EBOOK"
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

int isStopWord(char *line, int start, int end, word stopWords[NSTOPWORDS]);

int lineLength(char* line);

WordLine processLine(char* line, int length, word stopWords[NSTOPWORDS]);

void printWords(WordLine lineWords);

int main(int argc, char *argv[]) {
	int   nWords;    // Number of top frequency words to show
	char *fileName;  // Name of file containing book text

    FILE *sw;
    FILE *fp;

    word stopWords[NSTOPWORDS]; 

    Dict d = DictNew();

    // Read stopwords from file into array
    sw = fopen("stopwords", "r");

    if (sw == NULL) {
        fprintf(stderr, "Can't open stopwords\n");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    while (fgets(stopWords[i], MAXWORD+1, sw) != NULL) {
        // Remove newline character at end of each stopword
        int j = 0;
        while (stopWords[i][j] != '\n') {
            j++;
        }
        stopWords[i][j] = 0;
        i++;
    }

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

    fp = fopen(fileName, "r");

    if (fp == NULL) { // Handle case where filename in command argument can't be opened
        fprintf(stderr, "Can't open %s\n", fileName);
        exit(EXIT_FAILURE);
    }

    char line[MAXLINE]; // Array to store each line of the given file for processing
    int foundStart = FALSE; // Track whether the start line of a PG book was found in the file
    int foundEnd = FALSE; // Track whether the end line of a PG book was found in the file

    while (fgets(line, MAXLINE, fp) != NULL) {
        int len = lineLength(line); // Get length of the line in characters excluding newline
        if (foundStart == TRUE) { // Only process text between the PG start and end lines
            if (len >= ENDSTRINGLEN && strncmp(line, ENDSTRING, ENDSTRINGLEN) == 0) {
                foundEnd = TRUE; // PG end line has been found, end text processing
                break;
            }
            WordLine lineWords = processLine(line, len, stopWords); // Process and extract words from line
            for (int i = 0; i < lineWords->size; i++) {
                DictInsert(d, lineWords->words[i]); // Insert each processed word into the dict
            }
            FreeWordLine(lineWords);
        } else if (len >= STARTSTRINGLEN && strncmp(line, STARTSTRING, STARTSTRINGLEN) == 0) {
            foundStart = TRUE; // PG start line has been found, begin text processing
        }
    }

    if (foundStart == FALSE || foundEnd == FALSE) { // Never found start or end string of a PG book
        fprintf(stderr, "Not a Project Gutenberg book\n");
        exit(EXIT_FAILURE);
    } else { // Found both start and end strings, print requested number of WFreqs
        WFreq *wfs = malloc(sizeof(WFreq)*nWords);

        int nWordsFound = DictFindTopN(d, wfs, nWords); // Retrieve top n words by freq., in lex. order

        for (int i = 0; i < nWordsFound; i++) { // Print the n WFreqs in required format
            printf("%d %s\n", wfs[i].freq, wfs[i].word);
        }
        DictShow(d);
        free(wfs);
        DictFree(d);
    }
}

WordLine NewWordLine(void) {
    WordLine w = malloc(sizeof(*w));
    if (w == NULL) {
        fprintf(stderr, "couldn't allocate WordLine");
        exit(EXIT_FAILURE);
    }

    w->words = malloc(MAXWORD*sizeof(char));
    if (w->words == NULL) {
        fprintf(stderr, "couldn't allocate WordLine words");
        exit(EXIT_FAILURE);
    }

    w->size = 0;
    return w;
}

void FreeWordLine(WordLine w) {
    if (w != NULL) {
        if (w->words != NULL) {
            free(w->words);
        }
        free(w);
    }
}

int lineLength(char* line) {
    int length = 0;
    while (line[length] != '\n' && line[length] != 0) {
        length++;
    }
    return length;
}

// Check whether a given section of a line is a stopword
int isStopWord(char *line, int start, int end, word stopWords[NSTOPWORDS]) {
    word checkWord;
    int wordLength = end - start;
    memcpy(checkWord, line + start, wordLength); // Copy word from line into checkWord array
    checkWord[wordLength] = 0; // Add terminating character to end of word string

    for (int i = 0; i < NSTOPWORDS; i++) {
        int cmp = strcmp(checkWord, stopWords[i]);

        if (cmp < 0) { // Word comes before current stopword in lex. order, no further matches possible
            return FALSE;
        }

        if (cmp == 0) { // Exact match, word is in list of stopwords
            return TRUE;
        } 
    }
    return FALSE; // Word was not in list of stopwords
}

WordLine processLine(char* line, int length, word stopWords[NSTOPWORDS]) {
    WordLine lineWords = NewWordLine();

    int start = 0;
    int end = 0;

    while (end < length) {
        while (!isWordChar(line[start])) { // Find next valid word character, to be start of next word
            start++;
        }
        end = start+1;
        while (isWordChar(line[end]) && end < length) { // Find first non-word character to be end of next word
            end++;
        }
        int wordLength = end - start;
        if (wordLength > 1) { // Ignore single-character words
            for (int i = start; i < end; i++) { // Normalise word
                line[i] = tolower(line[i]);
            }

            if (isStopWord(line, start, end, stopWords) == FALSE) { // Skip adding stopwords to word list
                memcpy(lineWords->words[lineWords->size], line+start, wordLength); // Copy word from line array to word list
                lineWords->words[lineWords->size][wordLength] = 0; // Set end character of word to 0
                stem(lineWords->words[lineWords->size], 0, wordLength-1); // Stem newly added words
                lineWords->size++;
                // Increase size of words char array in lineWords to accommodate further words
                lineWords->words = realloc(lineWords->words, MAXWORD * sizeof(char) * (lineWords->size + 1));
            }
        }
        end++;
        start = end;
        end++;
    }
    return lineWords;
}

void printWords(WordLine lineWords) {
    for (int i = 0; i < lineWords->size; i++) {
        printf("Word %d: %s\n", i+1, lineWords->words[i]);
    }
}
