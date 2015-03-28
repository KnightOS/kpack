#include "unpack.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <errno.h>
#include <libgen.h>

void mkpath(const char *root, const char *_path) {
	char *path = malloc(strlen(_path));
	strcpy(path, _path);
	char *slash = path;
	char backup;
	char *fullPath;
	while (slash && *slash) {
		slash = strchr(slash, '/');
		if (slash != NULL) {
			backup = slash[1];
			slash[1] = '\0';
		}
		fullPath = malloc(strlen(root) + strlen(path) + 1);
		strcpy(fullPath, root);
		strcat(fullPath, path);
		if (0 != mkdir(fullPath, S_IRWXU)) {
			if (errno != EEXIST) {
				printf("Unable to create %s\n", fullPath);
				exit(1);
			}
		}
		free(fullPath);
		if (slash) {
			slash[1] = backup;
			slash++;
		}
	}
	free(path);
}

void unpack(FILE *file, const char *root, int write_stub) {
	char *name = NULL, *repo = NULL;
	FILE *stub = NULL;
	int major = -1, minor = -1, patch = -1;
	char magic[5] = { 0 };
	fread(magic, sizeof(char), sizeof(magic) / sizeof(char), file);
	if (strcmp(magic, "KPKG")) {
		printf("Error: this file is not a valid KnightOS package.\n");
		fclose(file);
		exit(1);
	}
	/* Process metadata */
	uint8_t keyslen = 0;
	fread(&keyslen, sizeof(uint8_t), 1, file);
	int i;
	for (i = 0; i < keyslen; ++i) {
		uint8_t vlen = 0;
		uint8_t key = fgetc(file);
		fread(&vlen, sizeof(uint8_t), 1, file);
		if (key == KEY_PKG_REPO) {
			repo = malloc(vlen + 1);
			fread(repo, sizeof(char), vlen, file);
			repo[vlen] = '\0';
		} else if (key == KEY_PKG_NAME) {
			name = malloc(vlen + 1);
			fread(name, sizeof(char), vlen, file);
			name[vlen] = '\0';
		} else if (key == KEY_PKG_VERSION) {
			major = fgetc(file);
			minor = fgetc(file);
			patch = fgetc(file);
		} else {
			fseek(file, vlen, SEEK_CUR); // Discard value
		}
	}
	if (name == NULL || repo == NULL || major == -1) {
		printf("Error: invalid package (name, repo, and version are all required).");
		exit(1);
	}
	printf("Extracting %s/%s:%d.%d.%d to %s\n", repo, name, major, minor, patch, root);
	uint8_t fileslen = 0;
	fread(&fileslen, sizeof(uint8_t), 1, file);
	if (write_stub) {
		long pos = ftell(file);
		fseek(file, 0, SEEK_SET);
		/* Make sure /var/packages/<repo>/ exists */
		char *stubdir = malloc(
			strlen(repo) +
			strlen("/var/packages/") +
			strlen(name) +
			strlen("-256.256.256.stub"));
		strcpy(stubdir, "/var/packages/");
		strcat(stubdir, repo);
		mkpath(root, stubdir);
		sprintf(stubdir + strlen(repo) + strlen("/var/packages/"), "/%s-%d.%d.%d.stub", name, major, minor, patch);
		char *fullpath = malloc(strlen(root) + strlen(stubdir));
		strcpy(fullpath, root);
		strcat(fullpath, stubdir);
		stub = fopen(fullpath, "wb");
		if (!stub) {
			printf("Unable to open %s for writing.\n", fullpath);
			exit(1);
		}
		free(stubdir);
		free(fullpath);
		char *buffer = malloc(pos);
		fread(buffer, sizeof(uint8_t), pos, file);
		fwrite(buffer, sizeof(uint8_t), pos, stub);
	}
	/* Extract files */
	for (i = 0; i < fileslen; ++i) {
		uint8_t pathlen = 0;
		fread(&pathlen, sizeof(uint8_t), 1, file);
		char *path = calloc(pathlen + 1, sizeof(char));
		fread(path, sizeof(uint8_t), pathlen, file);
		if (write_stub) {
			/* Copy file header to stub */
			fwrite(&pathlen, sizeof(uint8_t), 1, stub);
			fwrite(path, sizeof(uint8_t), pathlen, stub);
		}
		uint8_t compression;
		fread(&compression, sizeof(uint8_t), 1, file);
		uint32_t uncompressedlen = 0;
		//uint32_t compressedlen = 0;
		uncompressedlen = fgetc(file) | (fgetc(file) << 8) | (fgetc(file) << 16);
		//compressedlen = fgetc(file) | (fgetc(file) << 8) | (fgetc(file) << 16);
		fseek(file, 3, SEEK_CUR);
		/* Get the path for mkpath */
		char *dir = calloc(strlen(path) + 1, sizeof(char));
		strcpy(dir, path);
		int i;
		for (i = strlen(dir); i >= 0; --i) {
			if (dir[i] == '/') {
				dir[i] = '\0';
				break;
			}
		}
		char *outpath = calloc(strlen(dir) + 1, sizeof(char));
		strcpy(outpath, dir);
		int skip = 0;
		if (write_stub && strstr(dir, "/include") == dir) { /* Omit include files */
			skip = 1;
			printf("Omitting %s.\n", path);
		} else if (write_stub && strstr(dir, "/slib") == dir) { /* Omit static libraries */
			skip = 1;
			printf("Omitting %s.\n", path);
		} else {
			printf("Extracting %s...\n", path);
		}
		free(dir);
		if (!skip) {
			mkpath(root, outpath);
		}
		/* Write the file */
		char *base = calloc(strlen(path) + 1, sizeof(char));
		strcpy(base, path);
		base = basename(base);
		char *outfile = calloc(strlen(root) + strlen(outpath) + strlen(base) + 2, sizeof(char));
		strcpy(outfile, root);
		strcat(outfile, outpath);
		outfile[strlen(root) + strlen(outpath)] = '/';
		outfile[strlen(root) + strlen(outpath) + 1] = '\0';
		strcat(outfile, base);
		if (!skip) {
			FILE *output = fopen(outfile, "wb");
			if (!output) {
				printf("Unable to open %s for writing.\n", outfile);
				exit(1);
			}
			char *buffer = malloc(256);
			/* TODO: Compression */
			while (uncompressedlen > 0) {
				size_t len = uncompressedlen;
				if (len > 256) {
					len = 256;
				}
				len = fread(buffer, sizeof(char), len, file);
				fwrite(buffer, sizeof(char), len, output);
				uncompressedlen -= len;
			}
			fclose(output);
			free(buffer);
		} else {
			fseek(file, uncompressedlen, SEEK_CUR);
		}
		free(outpath);
		free(outfile);
		/* Checksum */
		uint8_t sumtype = 0;
		fread(&sumtype, sizeof(uint8_t), 1, file);
		if (sumtype == SUM_NONE) {
			/* nop */
		} else if (sumtype == SUM_CRC16) {
			fseek(file, 2, SEEK_CUR); /* TODO */
		} else if(sumtype == SUM_MD5) {
			fseek(file, 16, SEEK_CUR); /* TODO */
		} else if(sumtype == SUM_SHA1) {
			fseek(file, 20, SEEK_CUR); /* TODO */
		} else {
			printf("Error: unknown checksum type %02X.\n", sumtype);
			exit(1);
		}
		free(path);
	}
	if (write_stub) {
		fclose(stub);
	}
}
