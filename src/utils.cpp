#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "types.h"

char* read_entire_file(const char *path) {
	FILE *fp = fopen(path, "r");
	if(!fp) return 0;

	fseek(fp, 0L, SEEK_END);
	u32 size = ftell(fp);
	rewind(fp);

	char *data = (char*) calloc(size + 1, sizeof(char));
	if(!data) return 0;
	assert(fread(data, sizeof(char), size, fp) == size);

	fclose(fp);

	return data;
}