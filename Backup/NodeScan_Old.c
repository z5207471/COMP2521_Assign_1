WFreq wf = {.word = d->word, .freq = d->freq};
if (*s < n) {
    wfs[*s] = wf;
    (*s)++;
    if (*minFreq == 0 || wf.freq < *minFreq) {
        *minFreq = wf.freq;
    }
} else if (wf.freq == *minFreq) { // Word has same frequency as 
    int replIndex = -1;
    char replWord[MAXWORD] = { 0 };

    for (int i = 0; i < n; i++) {
        // Check if word has min. freq. and comes after word to be inserted
        if (wfs[i].freq == *minFreq &&
            ((replIndex == -1 && strcmp(wf.word, wfs[i].word) < 0) || 
                (strcmp(wf.word, wfs[i].word) < 0 && strcmp(wfs[i].word, replWord) > 0))) {
            replIndex = i; // Set word at current index to be replaced by word to be inserted
            strcpy(replWord, wfs[i].word);
        }
    }
    if (replIndex != -1) {
        wfs[replIndex] = wf;
    }
} else if (wf.freq > *minFreq) {
    int newMinFreq = wf.freq;
    int nAtMinFreq = 0;
    int replIndex = -1;
    char replWord[MAXWORD] = { 0 };

    for (int i = 0; i < n; i++) {
        // Check if word has min. freq. and comes after current replWord
        if (wfs[i].freq == *minFreq &&
            (replIndex == -1 || strcmp(wfs[i].word, replWord) > 0)) {
            nAtMinFreq++;
            replIndex = i; // Set word at current index to be replaced by word to be inserted
            strcpy(replWord, wfs[i].word);
        } else if (wfs[i].freq < newMinFreq) {
            newMinFreq = wfs[i].freq;
        }
    }
    wfs[replIndex] = wf;
    if (nAtMinFreq == 1) { // If inserted WFreq replaced only WFreq with current minFreq, update minFreq
        *minFreq = newMinFreq;
    }
}