// COMP2521 21T2 Assignment 1
// Dict.c ... implementation of the Dictionary ADT

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

#define REBAL_PERIOD 10000

typedef struct DictNode *DictLink;
typedef struct WFreq *WF;

struct DictRep {
    DictLink root;
    int insertCount;
    WFreq *wfs;
};

struct DictNode {
    char word[MAXWORD];
    int freq;
    int index;
    DictLink left;
    DictLink right;
};

// Function prototypes

DictLink DictLinkNew(char *word);

void NodeFree(DictLink d);

DictLink NodeInsert(DictLink d, char *word, Dict dict);

int NodeFind(DictLink d, char *word);

DictLink NodeRotateLeft(DictLink d);

DictLink NodeRotateRight(DictLink d);

int NodeAssignIndex(DictLink d, int i);

int DictAssignIndices(Dict d);

int NodeCount(DictLink d);

DictLink NodePartition(DictLink d, int i);

void DictRebalance(Dict d);

WFreq *NodeScan(DictLink d, WFreq *wfs, int n, int *s, int *minFreq);

int compareWFs(const void *w1, const void *w2);

void NodeShow(DictLink d);

// Functions

// Creates a new Dictionary
Dict DictNew(void) {
    Dict new = malloc(sizeof(*new));
    new->insertCount = 0;
    new->root = NULL;
    return new;
}

DictLink DictLinkNew(char *word) {
    DictLink new = malloc(sizeof(*new));

    strcpy(new->word, word);
    new->freq = 1;
    new->index = -1;
    new->left = NULL;
    new->right = NULL;
    return new;
}

void NodeFree(DictLink d) {
    if (d == NULL) {
        return;
    }
    
    NodeFree(d->left);
    NodeFree(d->right);
    free(d);
}

// Frees the given Dictionary
void DictFree(Dict d) {
    if (d == NULL) {
        return;
    }
    
    NodeFree(d->root);
    free(d);
}

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

// Inserts an occurrence of the given word into the Dictionary
void DictInsert(Dict d, char *word) {
    if (d->root == NULL) {
        d->root = DictLinkNew(word);
    } else {
        d->root = NodeInsert(d->root, word, d);
    }
    // d->insertCount++;
    // if (d->insertCount > REBAL_PERIOD) {
    //     DictRebalance(d);
    // }
}

DictLink NodeRotateLeft(DictLink d) {
    if (d == NULL || d->right == NULL) {
        return d;
    }
    DictLink newRoot = d->right;
    d->right = newRoot->left;
    newRoot->left = d;
    return newRoot;
}

DictLink NodeRotateRight(DictLink d) {
    if (d == NULL || d->left == NULL) {
        return d;
    }
    DictLink newRoot = d->left;
    d->left = newRoot->right;
    newRoot->right = d;
    return newRoot;
}

int NodeAssignIndex(DictLink d, int i) {
    if (d == NULL) {
        return i;
    }
    d->index = NodeAssignIndex(d->left, i);
    return NodeAssignIndex(d->right, i+1);
}

int DictAssignIndices(Dict d) {
    int maxIndex = 0;
    if (d->root != NULL) {
        maxIndex = NodeAssignIndex(d->root, 0) - 1;
    }
    return maxIndex;
}

int NodeCount(DictLink d) {
    if (d == NULL) {
        return 0;
    }
    return NodeCount(d->left) + NodeCount(d->right) + 1;
}

DictLink NodePartition(DictLink d, int i) {
    int leftSize = NodeCount(d->left);
    if (i < leftSize) {
        d->left = NodePartition(d->left, i);
        d = NodeRotateRight(d);
    } else if (i > leftSize) {
        d->right = NodePartition(d->right, i - leftSize - 1);
        d = NodeRotateLeft(d);
    }
    return d;
}

void DictRebalance(Dict d) {
    int midIndex = DictAssignIndices(d) / 2;
    d->root = NodePartition(d->root, midIndex);
    d->insertCount = 0;
}

int NodeFind(DictLink d, char *word) {
    if (d == NULL) {
        return false;
    } else if (strcmp(word, d->word) < 0) {
        return NodeFind(d->left, word);
    } else if (strcmp(word, d->word) > 0) {
        return NodeFind(d->right, word);
    } else { // (strcmp(word, n->word) == 0)
        return true;
    }
}

// Returns the occurrence count of the given word. Returns 0 if the word
// is not in the Dictionary.
int DictFind(Dict d, char *word) {
	if (d == NULL) {
        return false;
    }
    return NodeFind(d->root, word);
}

WFreq *NodeScan(DictLink d, WFreq *wfs, int n, int *s, int *minFreq) {
    if (d != NULL) {
        WFreq wf = {.word = d->word, .freq = d->freq};
        if (*s < n) {
            wfs[*s] = wf;
            (*s)++;
            if (*minFreq == 0 || wf.freq < *minFreq) {
                *minFreq = wf.freq;
            }
            if (*s == n) {
                qsort(wfs, n, sizeof(WFreq), compareWFs);
            }
        } else if (compareWFs(&wf, &wfs[n-1]) < 0) {
            wfs[n-1] = wf;
            qsort(wfs, n, sizeof(WFreq), compareWFs);
            *minFreq = wfs[n-1].freq;
        }
        NodeScan(d->left, wfs, n, s, minFreq);
        NodeScan(d->right, wfs, n, s, minFreq);
    }
    return wfs;
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

    wfs = NodeScan(d->root, wfs, n, &nStored, &minFreq);

    if (nStored < n) {
        qsort(wfs, nStored, sizeof(WFreq), compareWFs);
    }

    return nStored;
}

// Compare WFreqs by freq. if their freq.s differ, otherwise compare by lex. order
int compareWFs(const void *w1, const void *w2) {
    WFreq *wf1 = (WFreq *)w1;
    WFreq *wf2 = (WFreq *)w2;
    int fdiff = wf2->freq - wf1->freq;                        
    return(fdiff != 0 ? fdiff : strcmp(wf1->word, wf2->word));                              
}

void NodeShow(DictLink d) {
    if (d != NULL) {
        printf("Word: %s, Frequency: %d", d->word, d->freq);
        NodeShow(d->left);
        NodeShow(d->right);
    }
}

// Displays the given Dictionary. This is purely for debugging purposes,
// so  you  may  display the Dictionary in any format you want.  You may
// choose not to implement this.
void DictShow(Dict d) {
    printf("Printing dict\n");
    NodeShow(d->root);
}
