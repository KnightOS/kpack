#include "common.h"

void initRuntime() {
	packager.config = NULL;
	packager.configName = NULL;
	packager.pack = 1;
	packager.filename = NULL;
	packager.rootName = NULL;
	packager.pkgname = NULL;
	packager.output = NULL;
	packager.repo = NULL;
	packager.version.major = packager.version.minor = packager.version.patch = -1;
	packager.mdlen = 0;
	packager.compressionType = COMPRESSION_PUCRUNCH;
	packager.sumType = SUM_CRC16;
	packager.flen = 0;
}

#define match_option(short, opt) (strcmp(opt, argv[i]) == 0 || strcmp(short, argv[i]) == 0)
int parse_args(int argc, char **argv) {
	int i;
	argc--;
	argv++;
	if (!argc) {
		printf("Invalid usage.\n\n");
		displayUsage();
		return 1;
	}
	for (i = 0; i < argc; i++) {
		if (match_option("-c", "--config")) {
			i++;
			packager.configName = argv[i];
		} else if (match_option("-e", "--extract")) {
			packager.pack = 0;
		} else if (match_option("-s", "--sum")) {
			i++;
			if (strcmp(argv[i], "crc16") == 0) {
				packager.sumType = SUM_CRC16;
			} else if (strcmp(argv[i], "sha1") == 0) {
				packager.sumType = SUM_SHA1;
			} else if (strcmp(argv[i], "md5") == 0) {
				packager.sumType = SUM_MD5;
			} else if (strcmp(argv[i], "none") == 0) {
				packager.sumType = SUM_NONE;
			} else {
				printf("Invalid checksum type. Valid choices are 'crc16', 'sha1', 'md5' or 'none'.\n");
				return 1;
			}
		} else if (match_option("-x", "--compressor")) {
			i++;
			if (strcmp(argv[i], "pucrunch") == 0) {
				packager.compressionType = COMPRESSION_PUCRUNCH;
			} else if (strcmp(argv[i], "rle") == 0) {
				packager.compressionType = COMPRESSION_RLE;
			} else if (strcmp(argv[i], "none") == 0) {
				packager.compressionType = COMPRESSION_NONE;
			} else {
				printf("Invalid compression type. Valid choices are 'pucrunch', 'rle' or 'none'.\n");
				return 1;
			}
		} else {
			if (!packager.filename) {
				// That's the file's name
				packager.filename = argv[i];
			} else if (!packager.rootName) {
				// That's the model
				packager.rootName = argv[i];
			} else {
				// Hum ...
				printf("Unexpected extra argument.\n");
				return 1;
			}
		}
	}
	
	if (!packager.filename) {
		printf("Expected a package name, none found.\n");
		return 1;
	}
	if (!packager.rootName) {
		printf("Expected a model name, none found.\n");
	}
	
	if (!packager.configName) {
		packager.configName = "package.config";
	}
		
	packager.config = fopen(packager.configName, "r");
	
	if (!packager.config) {
		printf("Config file '%s' not found.\n", packager.configName);
		return 1;
	}
	
	return 0;
}

char *config_read_line() {
	char *result;
	int c;
	int size = 1, allocLimit = 128;
	
	result = malloc(allocLimit);
	
	while (1) {
		c = fgetc(packager.config);
		if (c == EOF || c == '\n') {
			result[size - 1] = '\0';
			break;
		}
		result[size - 1] = c;
		size++;
		if (size > allocLimit) {
			allocLimit += 128;
			result = realloc(result, allocLimit);
		}
	}
	return result;
}

int config_key_match(char *s, char *k) {
	while (*s != '=' && *s != '\0') {
		if (*s != *k) {
			break;
		}
		s++;
		k++;
	}
	return !*k;
}

char *config_get_string(char *s) {
	int size = 0;
	char *result;
	while (*s++ != '=');
	
	while (s[size++] != '\0');
	result = malloc(size);
	strcpy(result, s);
	
	return result;
}

inline void config_get_version(char *s, char delimiter, versionData *v) {
	while (*s++ != delimiter);
	sscanf(s, "%hhu.%hhu.%hhu", &v->major, &v->minor, &v->patch);
}

int parse_metadata() {
	int returnV = 0;
	int done = 0;
	char *line;
	
	do {
		do {
			line = config_read_line();
			if (*line == '\n' || *line == '#') {
				free(line);
				line = NULL;
			}
		} while (!line);
		
		// Required
		if (config_key_match(line, "name")) {
			packager.pkgname = config_get_string(line);
			printf("Parsed package name '%s'\n", packager.pkgname);
			packager.mdlen++;
		} else if (config_key_match(line, "repo")) {
			packager.repo = config_get_string(line);
			printf("Parsed package repo '%s'\n", packager.repo);
			packager.mdlen++;
		} else if (config_key_match(line, "version")) {
			config_get_version(line, '=', &packager.version);
			printf("Parsed package version '%hhu.%hhu.%hhu'\n", packager.version.major, packager.version.minor, packager.version.patch);
			packager.mdlen++;
		}
		// The rest is optional
		//
		// TODO
		//
		
		done = *line == '\0';
		free(line);
	} while (!done);
	
	if (!packager.pkgname) {
		printf("Couldn't parse package name !\n");
		returnV = 1;
	}
	if (!packager.repo) {
		printf("Couldn't parse package repo !\n");
		returnV = 1;
	}
	if (packager.version.major == 0xff || packager.version.minor == 0xff || packager.version.patch == 0xff) {
		printf("Couldn't parse version number !\n");
		returnV = 1;
	}
	
	return returnV;
}

void writeFileToPackage(char *f) {
	// Don't include the model's root name
	int size, c;
	char *filename = strchr(f, '/');
	FILE *in = fopen(f, "rb");
	int plen = strlen(filename);
	fputc(plen, packager.output);
	fputs(filename, packager.output);
	fputc(packager.compressionType, packager.output);
	fseek(in, 0, SEEK_END);
	size = ftell(in);
	fseek(in, 0, SEEK_SET);
	// 24-bits size
	fputc(size & 0xff, packager.output);
	fputc((size >> 8) & 0xff, packager.output);
	fputc((size >> 16) & 0xff, packager.output);
	if (packager.compressionType == COMPRESSION_RLE) {
		//
		// TODO
		//
		// RLE compression
		// put compressed size in variable 'size'
	} else if (packager.compressionType == COMPRESSION_PUCRUNCH) {
		//
		// TODO
		//
		// RLE compression
		// put compressed size in variable 'size'
	}
	// 24-bits size
	fputc(size & 0xff, packager.output);
	fputc((size >> 8) & 0xff, packager.output);
	fputc((size >> 16) & 0xff, packager.output);
	
	while ((c = getc(in)) != EOF) {
		fputc(c, packager.output);
	}
	
	if (packager.sumType == SUM_NONE) {
		fputc(SUM_NONE, packager.output);
	} else if (packager.sumType == SUM_CRC16) {
		fputc(SUM_CRC16, packager.output);
		//
		// TODO
		//
		// CRC16 checksum
		// directly write the thing
	} else if (packager.sumType == SUM_SHA1) {
		fputc(SUM_SHA1, packager.output);
		//
		// TODO
		//
		// SHA1 checksum
		// directly write the thing
	} else if (packager.sumType == SUM_MD5) {
		fputc(SUM_MD5, packager.output);
		//
		// TODO
		//
		// MD5 checksum
		// directly write the thing
	}
}

void printSpaces(int j) {
	int i;
	for(i = 0; i < j; i++) {
		printf("--");
	}
}

void writeModelRecursive(DIR *root, char *rootName, struct dirent *currentEntry, int indent) {
	DIR *rroot;
	struct dirent *rentry;
	char rrootName[1024];
	char rfilename[1024];
	
	while (currentEntry) {
		if (currentEntry->d_type == DT_REG) {
			// found a file, write it to output
			printSpaces(indent);
			sprintf(rfilename, "%s/%s", rootName, currentEntry->d_name);
			printf("Adding file %s to package ...\n", rfilename);
			writeFileToPackage(rfilename);
			currentEntry = readdir(root);
			
		} else if (currentEntry->d_type == DT_DIR) {
			// found a directory, recursively explore it
			// ... except if it's . or ..
			if (strcmp(currentEntry->d_name, ".") && strcmp(currentEntry->d_name, "..")) {
				sprintf(rrootName, "%s/%s", rootName, currentEntry->d_name);
				printSpaces(indent);
				printf("Entering directory %s.\n", rrootName);
				rroot = opendir(rrootName);
				if(!rroot) {
					printSpaces(indent);
					printf("Couldn't open dir %s\n", rrootName);
				} else {
					rentry = readdir(rroot);
					if(rentry) {
						writeModelRecursive(rroot, rrootName, rentry, indent + 1);
					}
					closedir(rroot);
				}
				currentEntry = readdir(root);
			} else {
				// in that case, skip it
				currentEntry = readdir(root);
			}
		}
	}
	printSpaces(indent);
	printf("No more files in directory %s.\n\n", rootName);
}

void writeModel(DIR *root, char *rootName) {
	struct dirent *currentEntry;
	
	currentEntry = readdir(root);
	if(currentEntry) {	
		writeModelRecursive(root, rootName, currentEntry, 0);
	} else {
		printf("Model %s is empty !\nAborting operation.\n", rootName);
	}
}
