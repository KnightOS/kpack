#ifndef INC_COM
#define INC_COM

#include <stdio.h>
#include <ctype.h>
#include <string.h>

typedef struct {
	uint8_t major;
	uint8_t minor;
	uint8_t patch;
} versionData;

typedef struct {
	int included;
	versionData version;
	uint8_t nameLength;
	char *name;
} packageDependency;

typedef struct {
	uint8_t key;
	uint8_t vlen;
	void *value;
} metadata;

typedef struct {
	uint8_t pathLength;
	char *path;
	// Only 24 bits are used
	uint32_t uncompressedLength;
	uint32_t compressedLength;
	FILE *file;
	char *sumString;
	uint16_t crc16;
} filedata;

struct {
	// Runtime
	FILE *config;
	char *configName;
	int pack;
	char *rootName;
	//
	// Package
	//
	char *name;
	uint8_t version;
	// Metadata
	uint8_t mdlen;
	metadata **md;
	// Files
	uint8_t compressionType;
	uint8_t sumType;
	uint8_t flen;
	filedata **files;
} packager;

struct {
	uint8_t mdlen;
	metadata **md;
	// This doesn't include the files' contents
	filedata **files;
} packageStub;

enum {
	KEY_PKG_NAME,
	KEY_PKG_REPO,
	KEY_PKG_DESCRIPTION,
	KEY_PKG_DEPS,
	KEY_PKG_VERSION,
	KEY_PKG_AUTHOR,
	KEY_PKG_MAINTENER,
	KEY_PKG_COPYRIGHT,
	KEY_INFO_URL
};

enum {
	COMPRESSION_NONE,
	COMPRESSION_RLE,
	COMPRESSION_PUCRUNCH
};

enum {
	SUM_NONE,
	SUM_CRC16,
	SUM_SHA1,
	SUM_MD5
};

extern void displayUsage();
#endif
