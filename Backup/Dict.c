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

typedef struct DictNode *DictLink;
typedef struct WFreq *WF;

struct DictRep {
    DictLink root;
    int size;
};

struct DictNode {
    char word[MAXWORD];
    int freq;
    int height;
    DictLink left;
    DictLink right;
};

// Function prototypes

DictLink DictLinkNew(char *word);

void NodeFree(DictLink d);

int NodeHeight(DictLink d);

void NodeUpdateHeight(DictLink d);

int NodeBal(DictLink d);

DictLink NodeRotateLeft(DictLink root);

DictLink NodeRotateRight(DictLink d);

DictLink NodeInsert(DictLink d, char *word);

int NodeFind(DictLink d, char *word);

WFreq *NodeScan(DictLink d, WFreq *wfs, int n, int *s, int *minFreq);

void NodeShow(DictLink d);

// Functions

int max(int a, int b)
{
    return (a > b) ? a : b;
}

// Creates a new Dictionary
Dict DictNew(void) {
    Dict new = malloc(sizeof(*new));

    new->root = NULL;
    new->size = 0;
    return new;
}

DictLink DictLinkNew(char *word) {
    DictLink new = malloc(sizeof(*new));

    strcpy(new->word, word);
    new->freq = 1;
    new->height = 1;
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

int NodeHeight(DictLink d) {
    if (d == NULL) {
        return 0;
    }
    return d->height;
}

void NodeUpdateHeight(DictLink d) {
    d->height = max(NodeHeight(d->left), NodeHeight(d->right)) + 1;
}

int NodeBal(DictLink d) {
    if (d == NULL)
        return 0;
    return NodeHeight(d->left) - NodeHeight(d->right);
}

DictLink NodeRotateLeft(DictLink root) {
    if (root == NULL || root->right == NULL) {
        return root;
    }

    DictLink newRoot = root->right;

    // Rotate
    root->right = newRoot->left;
    newRoot->left = root;

    // Update heights
    NodeUpdateHeight(root);
    NodeUpdateHeight(newRoot);

    return newRoot;
}

DictLink NodeRotateRight(DictLink root) {
    if (root == NULL || root->left == NULL) {
        return root;
    }

    DictLink newRoot = root->left;

    // Rotate
    root->left = newRoot->right;
    newRoot->right = root;

    // Update heights
    NodeUpdateHeight(root);
    NodeUpdateHeight(newRoot);

    return newRoot;
}

DictLink NodeInsert(DictLink d, char *word) {
    if (d == NULL) {
        return DictLinkNew(word);
    } else if (strcmp(word, d->word) < 0) {
        d->left = NodeInsert(d->left, word);
    } else if (strcmp(word, d->word) > 0) {
        d->right = NodeInsert(d->right, word);
    } else { // (strcmp(word, n->word) == 0)
        d->freq++;
        return d;
    }

    NodeUpdateHeight(d);

    int bal = NodeBal(d);

    if (bal > 1) {
        if (strcmp(word, d->word) < 0) { // Left Left Case
            return NodeRotateRight(d);
        } else if (strcmp(word, d->word) > 0) { // Left Right Case
            d->left =  NodeRotateLeft(d->left);
            return NodeRotateRight(d);
        }
    } else if (bal < -1) {
        if (strcmp(word, d->word) < 0) { // Right Left Case
            d->right = NodeRotateRight(d->right);
            return NodeRotateLeft(d);
        } else if (strcmp(word, d->word) > 0) { // Right Right Case
            return NodeRotateRight(d);
        }
    }

    return d;
}

// Inserts an occurrence of the given word into the Dictionary
void DictInsert(Dict d, char *word) {
    if (d->root == NULL) {
        d->root = DictLinkNew(word);
        d->size++;
    } else {
        d->root = NodeInsert(d->root, word);
    }
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
	if (d == NULL || d->size == 0) {
        return false;
    }
    return NodeFind(d->root, word);
}

WFreq *NodeScan(DictLink d, WFreq *wfs, int n, int *s, int *minFreq) {
    if (d != NULL) {
        if (*s < n) { // WFreq array is not yet full, add any word
            WFreq wf = { .word = d->word, .freq = d->freq };
            wfs[*s] = wf;
            if (wf.freq < *minFreq) {
                *minFreq = wf.freq;
            }
            (*s)++;
        } else if (d->freq > *minFreq) { // WFreq array is full, only insert most frequent words
            int newMinFreq = d->freq;
            int replIndex = -1;
            char replWord[MAXWORD] = {0};

            WFreq wf = { .word = d->word, .freq = d->freq };

            for (int i = 0; i < n; i++) { // Replace min freq word with new word. Find new min freq.
                if (wfs[i].freq == *minFreq && (replWord[0] == 0 || strcmp(wfs[i].word, replWord) > 0)) {
                    replIndex = i;
                    strcpy(replWord, wfs[i].word);
                } else if (wfs[i].freq < newMinFreq) {
                    newMinFreq = wfs[i].freq;
                }
            }

            if (replIndex != -1) {
                wfs[replIndex] = wf;
                *minFreq = newMinFreq;
            }
        } else if (d->freq == *minFreq) {
            WFreq wf = { .word = d->word, .freq = d->freq };

            for (int i = 0; i < n; i++) { // Replace min freq word with new word. Find new min freq.
                if (wfs[i].freq == *minFreq && strcmp(wfs[i].word, wf.word) > 0) {
                    wfs[i] = wf;
                    break;
                }
            }
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

    int swapped = TRUE;
    while (swapped == TRUE) {
        swapped = FALSE;
        WFreq temp;
        for (int i = 0; i < nStored-1; i++) {
            if (wfs[i].freq < wfs[i+1].freq || 
                (wfs[i].freq == wfs[i+1].freq && strcmp(wfs[i].word, wfs[i+1].word) > 0)) {
                memcpy(&temp, &wfs[i], sizeof(wfs[i]));
                memcpy(&wfs[i], &wfs[i+1], sizeof(wfs[i+1]));
                memcpy(&wfs[i+1], &temp, sizeof(wfs[i+1]));
                swapped = TRUE;
            }
        }
    }

    return nStored;
}

void NodeShow(DictLink d) {
    if (d != NULL) {
        printf("Word: %s, Frequency: %d, Height: %d\n", d->word, d->freq, d->height);
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
