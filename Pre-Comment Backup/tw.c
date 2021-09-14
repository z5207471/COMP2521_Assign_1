// COMP2521 21T2 Assignment 1
//
// tw.c ... compute top N most frequent words in file F
//
// Overview: Reads number of required words (optional, defaults to 10) and 
//           filename as arguments from stdin. Assuming that the input file
//           is a Project Gutenberg book in plain text format, reads contents
//           of the book only. Each line is tokenised and normalised, then
//           single character words and stopwords are discarded and the
//           remaining words are stemmed.
//
// Author: Lachlan Scott (z5207471
//
// Written: 21/6/21 - 16/7/21
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

void printWords(WordLine lineWords);

int main(int argc, char *argv[]) {
    int   nWords;    // Number of top frequency words to show
    char *fileName;  // Name of file containing book text

    FILE *fp;

    WordLine *stopWords = getStopWords();
    
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

    Dict d = DictNew();
    char line[MAXLINE+1]; // Array to store each line of the given file for processing
    int foundStart = FALSE; // Track whether the start line of a PG book was found in the file
    int foundEnd = FALSE; // Track whether the end line of a PG book was found in the file

    while (fgets(line, MAXLINE+1, fp) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        int len = strlen(line); // Get length of the line in characters excluding newline
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
    fclose(fp);

    for (int i = 0; i < SWLENRANGE; i++) { // Free all stopword arrays
        FreeWordLine(stopWords[i]);
    }
    free(stopWords);

    if (foundStart == FALSE || foundEnd == FALSE) { // Never found start or end string of a PG book
        DictFree(d);
        fprintf(stderr, "Not a Project Gutenberg book\n");
        exit(EXIT_FAILURE);
    } else { // Found both start and end strings, print requested number of WFreqs
        WFreq *wfs = malloc(sizeof(WFreq)*nWords);

        int nWordsFound = DictFindTopN(d, wfs, nWords); // Retrieve top n words by freq., in lex. order

        for (int i = 0; i < nWordsFound; i++) { // Print the n WFreqs in required format
            printf("%d %s\n", wfs[i].freq, wfs[i].word);
        }
        free(wfs);
        DictFree(d);
    }
}

WordLine NewWordLine(void) {
    WordLine w = (WordLine)malloc(sizeof(*w));
    if (w == NULL) {
        fprintf(stderr, "couldn't allocate WordLine");
        exit(EXIT_FAILURE);
    }

    w->words = malloc(MAXWORD * sizeof(char) * MAXLINEWORDS);
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

void WLAppendWord(WordLine l, char* w, int length) {
    memcpy(l->words[l->size], w, length); // Copy word to word list
    l->words[l->size][length] = 0; // Set end character of word to 0
    l->size++;
}

WordLine *getStopWords() {
    FILE *sw;

    // Initialise the stopwords array of wordline structs (stores stopwords sorted by length)
    WordLine *stopWords = (WordLine *)malloc(sizeof(WordLine) * SWLENRANGE);
    for (int i = 0; i < SWLENRANGE; i++) {
        stopWords[i] = NewWordLine();
    }

    // Read stopwords from file into array
    sw = fopen("stopwords", "r");

    if (sw == NULL) {
        fprintf(stderr, "Can't open stopwords\n");
        exit(EXIT_FAILURE);
    }

    word stopWord;

    while (fgets(stopWord, MAXWORD+1, sw) != NULL) { // Collect all stopwords into 2D stopwords array
        int l = 0;
        while (stopWord[l] != '\n') { // Get length of stopword
            l++;
        }
        if (l > 1) { // Only store stopwords longer than one character, since single char words are filtered out separately
            WLAppendWord(stopWords[l-2], stopWord, l);
        }
    }
    fclose(sw);
    return stopWords;
}

// Check whether a given section of a line is a stopword
int isStopWord(char *line, int start, int end, WordLine *stopWords) {
    int length = end - start;
    if (length > SWLENRANGE + 1) { // If word is longer than the longest stopword, match is impossible
        return FALSE;
    }
    word checkWord;
    memcpy(checkWord, line + start, length); // Copy word from line into checkWord array
    checkWord[length] = 0; // Add terminating character to end of word string

    for (int i = 0; i < stopWords[length-2]->size; i++) { // Iterate through stopwords of the same length
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

void normalise(char* line, int start, int end) {
    for (int i = start; i < end; i++) { // Normalise word
        line[i] = tolower(line[i]);
    }
}

WordLine processLine(char line[MAXLINE], int length, WordLine *stopWords) {
    WordLine lineWords = NewWordLine();

    if (length == 0) {
        return lineWords;
    }

    int start = 0;
    int end = 0;
    
    while (start < length) {
        while (start < length && !isWordChar(line[start])) { // Find next valid word character, to be start of next word
            start++;
        }
        end = start+1;
        while (end < length && isWordChar(line[end])) { // Find first non-word character to be end of next word
            end++;
        }
        int wordLength = end - start;
        if (wordLength > 1) { // Ignore single-character words
            normalise(line, start, end);

            if (isStopWord(line, start, end, stopWords) == FALSE) { // Skip adding stopwords to word list
                WLAppendWord(lineWords, line+start, wordLength);
                stem(lineWords->words[lineWords->size-1], 0, wordLength-1); // Stem newly added words
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
