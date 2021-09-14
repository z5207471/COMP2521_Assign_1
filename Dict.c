// COMP2521 21T2 Assignment 1
//
// Summary: Dict.c ... implementation of the Dictionary ADT
//
// Author:   Lachlan Scott (z5207471
//
// Written:  21/6/21 - 16/7/21

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Dict.h"
#include "WFreq.h"

#define MAXWORD 100

#define TRUE 1
#define FALSE 0

typedef struct DictNode *DictLink;
typedef struct WFreq *WF;

struct DictRep {
    DictLink root;
    WFreq *wfs;
};

struct DictNode {
    char word[MAXWORD];
    int freq;
    DictLink left;
    DictLink right;
};

// Function prototypes

DictLink DictLinkNew(char *word);

void NodeFree(DictLink d);

DictLink NodeInsert(DictLink d, char *word, Dict dict);

int NodeFind(DictLink d, char *word);

void NodeFindTopN(DictLink d, WFreq *wfs, int n, int *s, int *minFreq);

int compareWFs(const void *w1, const void *w2);

void NodeShow(DictLink d);

// Functions

// Creates a new Dictionary
Dict DictNew(void) {
    Dict new = malloc(sizeof(*new));
    new->root = NULL;
    return new;
}

// Creates a new DictLink pointer to a dict node
DictLink DictLinkNew(char *word) {
    DictLink new = malloc(sizeof(*new));

    strcpy(new->word, word);
    new->freq = 1;
    new->left = NULL;
    new->right = NULL;
    return new;
}

// Frees a dict node
// Does nothing if the given DictLink pointer to it is NULL
void NodeFree(DictLink d) {
    if (d == NULL) {
        return;
    }
    
    NodeFree(d->left);
    NodeFree(d->right);
    free(d);
}

// Frees the given Dictionary
// Does nothing if the pointer to the dictionary is NULL
void DictFree(Dict d) {
    if (d == NULL) {
        return;
    }
    
    NodeFree(d->root);
    free(d);
}

// Recursively inserts a given word into the correct dict node
DictLink NodeInsert(DictLink d, char *word, Dict dict) {
    if (d == NULL) {
        return DictLinkNew(word);
    } else if (strcmp(word, d->word) < 0) {
        d->left = NodeInsert(d->left, word, dict);
    } else if (strcmp(word, d->word) > 0) {
        d->right = NodeInsert(d->right, word, dict);
    } else { // (strcmp(word, n->word) == 0)
        d->freq++;
    }
    return d;
}

// Inserts an occurrence of the given word into the dictionary
void DictInsert(Dict d, char *word) {
    if (d->root == NULL) {
        d->root = DictLinkNew(word);
    } else {
        d->root = NodeInsert(d->root, word, d);
    }
}

// Recursively searches for a given word in the dictionary
// Returns TRUE if found, FALSE if not
int NodeFind(DictLink d, char *word) {
    if (d == NULL) {
        return FALSE;
    } else if (strcmp(word, d->word) < 0) {
        return NodeFind(d->left, word);
    } else if (strcmp(word, d->word) > 0) {
        return NodeFind(d->right, word);
    } else { // (strcmp(word, n->word) == 0)
        return d->freq;
    }
}

// Returns the occurrence count of the given word. 
// Returns FALSE if not found or dict is NULL
int DictFind(Dict d, char *word) {
	if (d == NULL) {
        return FALSE;
    }
    return NodeFind(d->root, word);
}

// Recursively finds the top n most frequent words in a dictionary
// sorted first by frequency, then by lexicographical order.
void NodeFindTopN(DictLink d, WFreq *wfs, int n, int *s, int *minFreq) {
    // If a node is empty, don't alter wfs array
    if (d != NULL) {
        WFreq wf = {.word = d->word, .freq = d->freq};

        // Case where wfs array is not full
        if (*s < n) {
            wfs[*s] = wf;
            (*s)++;
            
            if (*minFreq == 0 || wf.freq < *minFreq) {
                *minFreq = wf.freq;
            }

            // If wfs array was just filled, sort its contents
            if (*s == n) {
                qsort(wfs, n, sizeof(WFreq), compareWFs);
            }
        } else if (compareWFs(&wf, &wfs[n-1]) < 0) {
            // Case where wfs array is full and word should replace bottom word
            // Replace bottom word, then re-sort array
            wfs[n-1] = wf;
            qsort(wfs, n, sizeof(WFreq), compareWFs);
            *minFreq = wfs[n-1].freq;
        }
        NodeFindTopN(d->left, wfs, n, s, minFreq);
        NodeFindTopN(d->right, wfs, n, s, minFreq);
    }
}

// Finds  the top `n` frequently occurring words in the given Dictionary
// and stores them in the given  `wfs`  array  in  decreasing  order  of
// frequency,  and then in increasing lexicographic order for words with
// the same frequency. Returns the number of WFreq's stored in the given
// array (this will be min(`n`, #words in the Dictionary)) in  case  the
// Dictionary  does  not  contain enough words to fill the entire array.
// Assumes that the `wfs` array has size `n`.
int DictFindTopN(Dict d, WFreq *wfs, int n) {
    int minFreq = 0;
    int nStored = 0;

    NodeFindTopN(d->root, wfs, n, &nStored, &minFreq);

    // Fewer words stored than size of array, sort words before returning
    if (nStored < n) {
        qsort(wfs, nStored, sizeof(WFreq), compareWFs);
    }

    return nStored;
}

// Compare WFreqs by freq. if freq.s differ, otherwise compare by lex. order
// Returns value < 0 if first word has higher priority than second word,
// value > 0 if second word has higher priority and 0 if both words have
// same priority (identical word and frequency)
int compareWFs(const void *w1, const void *w2) {
    WFreq *wf1 = (WFreq *)w1;
    WFreq *wf2 = (WFreq *)w2;

    int fdiff = wf2->freq - wf1->freq;     

    return(fdiff != 0 ? fdiff : strcmp(wf1->word, wf2->word));                              
}

// Recursively prints the contents of each node in a dictionary
void NodeShow(DictLink d) {
    if (d != NULL) {
        printf("Word: %s, Frequency: %d", d->word, d->freq);
        NodeShow(d->left);
        NodeShow(d->right);
    }
}

// Prints the contents of the given Dictionary.
void DictShow(Dict d) {
    printf("Printing dict\n");
    NodeShow(d->root);
}
