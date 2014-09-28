#include "unpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void unpack(FILE *file, char *root) {
	char magic[5] = { 0 };
	fread(magic, sizeof(char), sizeof(magic) / sizeof(char), file);
	if (strcmp(magic, "KPKG")) {
		printf("Error: this file is not a valid KnightOS package.\n");
		fclose(file);
		exit(1);
	}
	/* Skip metadata */
	uint8_t keyslen = 0;
	fread(&keyslen, sizeof(uint8_t), 1, file);
	int i;
	for (i = 0; i < keyslen; ++i) {
		uint8_t vlen = 0;
		fgetc(file); // Discard key
		fread(&vlen, sizeof(uint8_t), 1, file);
		fseek(file, vlen, SEEK_CUR);
	}
	/* Extract files */
	uint8_t fileslen = 0;
	fread(&fileslen, sizeof(uint8_t), 1, file);
	for (i = 0; i < fileslen; ++i) {
		uint8_t pathlen = 0;
		fread(&pathlen, sizeof(uint8_t), 1, file);
		char *path = malloc(pathlen);
		fread(path, sizeof(uint8_t), pathlen, file);
		printf("Extracting %s...\n", path);
		exit(1);
	}
}
