// Program to test the Dictionary ADT

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "Dict.h"
#include "WFreq.h"

int main(void) {
	Dict d = DictNew();
	DictInsert(d, "hello");
	assert(DictFind(d, "hello") == 1);
	
	// add more tests here
	
	DictFree(d);
}

