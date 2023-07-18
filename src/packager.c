#include <errno.h>
#include <sys/stat.h>

#include "common.h"
#include "checksums.h"
#include "version.h"

void initRuntime() {
	packager.config = NULL;
	packager.configName = NULL;
	packager.pack = 1;
	packager.stub = 0;
	packager.printMeta = 0;
	packager.fileNb = 0;
	packager.filename = NULL;
	packager.rootName = NULL;
	packager.mdlen = 0;
	packager.compressionType = COMPRESSION_NONE;
	packager.sumType = SUM_CRC16;
	packager.flen = 0;
	packager.output = NULL;
	/* required metadata */
	packager.pkgname = NULL;
	packager.repo = NULL;
	packager.version.major = packager.version.minor = packager.version.patch = -1;
	/* optional metadata */
	packager.description = NULL;
	packager.dependencies_len = 0;
	packager.dependencies = malloc(sizeof(packageDependency*) * 128);
	packager.author = NULL;
	packager.maintainer = NULL;
	packager.infourl = NULL;
	packager.copyright = NULL;
}

#define match_option(short, opt) (strcmp(opt, argv[i]) == 0 || strcmp(short, argv[i]) == 0)
int parse_args(int argc, char **argv) {
	int i;
	argc--;
	argv++;
	if (!argc) {
		printf("Invalid usage.\n\n");
		displayUsage();
		return -1;
	}
	for (i = 0; i < argc; i++) {
		if (match_option("-c", "--config")) {
			i++;
			packager.configName = argv[i];
		} else if (match_option("-e", "--extract")) {
			packager.pack = 0;
		} else if (match_option("-s", "--stub")) {
			packager.stub = 1;
		} else if (match_option("-h", "--help")) {
			displayUsage();
			return 1;
		} else if (match_option("-i", "--info")) {
			packager.pack = 0;
			packager.printMeta = 1;
		} else if (match_option("-k", "--sum")) {
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
				printf("Invalid checksum type. See kpack --help.\n");
				return -1;
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
				printf("Invalid compression type. See kpack --help.\n");
				return -1;
			}
		} else if(match_option("-v", "--version")) {
			printf("kpack (KnightOS) %s\n", GIT_SHORT_VERSION);
			printf(COPYRIGHT);
			printf(LICENSE);
			return 1;
		} else if (*argv[i] == '-') {
			printf("Unknown option %s. See kpack --help for usage.\n", argv[i]);
			return -1;
		} else {
			if (!packager.filename) {
				packager.filename = argv[i];
			} else if (!packager.rootName) {
				packager.rootName = argv[i];
				if (packager.rootName[strlen(packager.rootName) - 1] == '/') {
					/* Remove trailing slash */
					packager.rootName[strlen(packager.rootName) - 1] = 0;
				}
			} else {
				printf("Error: unrecognized argument %s.\n", argv[i]);
				return -1;
			}
		}
	}
	
	if (!packager.filename) {
		printf("Invalid usage - no package specified. See kpack --help.\n");
		return -1;
	}
	if (!packager.rootName && !packager.printMeta) {
		/* if we are going to unpack then we need a target dir */
		if(!packager.pack) {
			printf("Invalid usage - no target directory specified for unpacking. See kpack --help.\n");
			return -1;
		}
		printf("Invalid usage - no model specified. See kpack --help.\n");
	}
	
	if(!packager.printMeta && packager.pack) {
		if (!packager.configName) {
			packager.configName = "package.config";
		}
			
		packager.config = fopen(packager.configName, "r");
		
		if (!packager.config) {
			printf("Config file '%s' not found.\n", packager.configName);
			return -1;
		}
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
			if (size == 1) {
				result[size - 1] = c == EOF ? '\0' : c;
				result[size] = '\0';
			} else {
				result[size - 1] = '\0';
			}
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
	return !*k && *s == '=';
}

char *config_get_string(char *s) {
	//int size = 0;
	char *result;
	while (*s++ != '=');
	
	result = malloc(strlen(s) + 1);
	strcpy(result, s);
	
	return result;
}

void config_get_version(char *s, char delimiter, versionData *v) {
	uint16_t major, minor, patch;
	while (*s++ != delimiter);
	sscanf(s, "%hu.%hu.%hu", &major, &minor, &patch);
	v->major = (uint8_t) major;
	v->minor = (uint8_t) minor;
	v->patch = (uint8_t) patch;
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
		
		// Required metadata
		if (config_key_match(line, "name")) {
			packager.pkgname = config_get_string(line);
			packager.mdlen++;
		} else if (config_key_match(line, "repo")) {
			packager.repo = config_get_string(line);
			packager.mdlen++;
		} else if (config_key_match(line, "version")) {
			config_get_version(line, '=', &packager.version);
			packager.mdlen++;
		}
		// Optional metadata
		else if (config_key_match(line, "description")) {
			packager.description = config_get_string(line);
			packager.mdlen++;
		} else if (config_key_match(line, "dependencies")) {
			// A bit more complicated
			packager.mdlen++;
			char *deplist = config_get_string(line);
			int i, j = 0;
			for (i = 0; 1; ++i) {
				if (deplist[i] == ' ' || deplist[i] == '\0') {
					char *dep = malloc(strlen(deplist));
					strcpy(dep, deplist + j);
					dep[i - j] = '\0';
					j = i + 1;
					versionData *version = malloc(sizeof(version));
					version->major = version->minor = version->patch = 0;
					if (strchr(dep, ':') != NULL) {
						config_get_version(dep, ':', version);
						*strchr(dep, ':') = '\0';
					}
					packageDependency *depv = malloc(sizeof(packageDependency));
					depv->version = *version;
					depv->name = dep;
					packager.dependencies[packager.dependencies_len] = depv;
					packager.dependencies_len++;
				}
				if (deplist[i] == '\0') {
					break;
				}
			}
		} else if (config_key_match(line, "author")) {
			packager.author = config_get_string(line);
			packager.mdlen++;
		} else if (config_key_match(line, "maintainer")) {
			packager.maintainer = config_get_string(line);
			packager.mdlen++;
		} else if (config_key_match(line, "copyright")) {
			packager.copyright = config_get_string(line);
			packager.mdlen++;
		} else if (config_key_match(line, "infourl")) {
			packager.infourl = config_get_string(line);
			packager.mdlen++;
		}
		
		done = *line == '\0';
		free(line);
	} while (!done);
	
	if (!packager.pkgname) {
		printf("Error: configuration does not include package name.\n");
		exit(1);
	}
	if (!packager.repo) {
		printf("Error: configuration does not include package repository.\n");
		exit(1);
	}
	if (packager.version.major == 0xff || packager.version.minor == 0xff || packager.version.patch == 0xff) {
		printf("Error: configuration does not include valid package version.\n");
		exit(1);
	}
	
	return returnV;
}

void writeFileToPackage(char *f, char *relpath) {
	char *filename = relpath;
	uint16_t crcsum;
	// Uncompressed then compressed size
	int usize = 0, csize = 0, c;
	FILE *in = fopen(f, "rb");
	int plen = strlen(filename);
	
	fputc(plen, packager.output);
	fputs(filename, packager.output);
	fputc(packager.compressionType, packager.output);
	
	fseek(in, 0, SEEK_END);
	usize = ftell(in);
	fseek(in, 0, SEEK_SET);
	// 24-bits uncompressed file's size
	fputc(usize & 0xff, packager.output);
	fputc((usize >> 8) & 0xff, packager.output);
	fputc((usize >> 16) & 0xff, packager.output);
	
	if (packager.compressionType == COMPRESSION_NONE) {
		csize = usize;
	} else if (packager.compressionType == COMPRESSION_RLE) {
		//
		// TODO
		//
		// RLE compression
		// put compressed size in variable 'csize'
	} else if (packager.compressionType == COMPRESSION_PUCRUNCH) {
		//
		// TODO
		//
		// pucrunch compression
		// put compressed size in variable 'csize'
	}
	// 24-bits compressed file's size
	fputc(csize & 0xff, packager.output);
	fputc((csize >> 8) & 0xff, packager.output);
	fputc((csize >> 16) & 0xff, packager.output);
	
	while ((c = fgetc(in)) != EOF) {
		fputc(c, packager.output);
	}
	
	if (packager.sumType == SUM_NONE) {
		// No checksum
		fputc(SUM_NONE, packager.output);
	} else if (packager.sumType == SUM_CRC16) {
		// CRC-16 checksum
		// See checksums.c for implementation
		fputc(SUM_CRC16, packager.output);
		crcsum = calculateCRC16(in);
		fputc(crcsum & 0xff, packager.output);
		fputc(crcsum >> 8, packager.output);
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

	/* clean up input file */
	if(in && fclose(in)) {
		printf("Error: Closed %s and this happened:\n\t%s\n", filename, strerror(errno));
	}
}

void writeModelRecursive(DIR *root, char *rootName, char* top, struct dirent *currentEntry) {
	DIR *rroot;
	struct dirent *rentry;
	struct stat s;
	int rrootNameL;
	char *rrootName;
	int rfilenameL;
	char *rfilename;
	
	while (currentEntry) {
		errno = 0;
		rfilenameL = strlen(rootName) + strlen(currentEntry->d_name) + 2;
		rfilename = malloc(rfilenameL * sizeof(char));
		sprintf(rfilename, "%s/%s", rootName, currentEntry->d_name);
		int status = stat(rfilename, &s);
		if (status != 0) {
			fprintf(stderr, "Error checking file %s: %d\n", currentEntry->d_name, errno);
			exit(1);
		}
		if (S_ISREG(s.st_mode)) {
			// found a file, write it to output
			packager.fileNb++;
			char *relpath = malloc((strlen(rfilename) - strlen(top)) + 1);
			strcpy(relpath, rfilename + strlen(top));
			/* make sure we don't attempt to include the output file itself */			
			if(!strncmp(packager.filename, (relpath + 1), strlen(packager.filename))) {
				printf("Ignoring file %s : This is most likely your output .pkg file.\n", relpath);
			} else {
				printf("Adding %s...\n", relpath);
				writeFileToPackage(rfilename, relpath);
			}
		} else if (S_ISDIR(s.st_mode)) {
			// found a directory, recursively explore it
			if (strcmp(currentEntry->d_name, ".") && strcmp(currentEntry->d_name, "..")) {
				rrootNameL = strlen(rootName) + strlen(currentEntry->d_name) + 1;
				rrootName = malloc(rrootNameL * sizeof(char) + 1);
				sprintf(rrootName, "%s/%s", rootName, currentEntry->d_name);
				rroot = opendir(rrootName);
				if(!rroot) {
					printf("Error: couldn't open %s\n", rrootName);
					free(rrootName);
					exit(1);
				} else {
					rentry = readdir(rroot);
					if(rentry) {
						writeModelRecursive(rroot, rrootName, top, rentry);
					}
					closedir(rroot);
					free(rrootName);
				}
			}
		} else {
			/* unknown entry, so move along!  */
			printf("Ignoring file : %s/%s\n", rootName, currentEntry->d_name);
		}
		free(rfilename);
		currentEntry = readdir(root);
	}
}

void writeModel(DIR *root, char *rootName) {
	int fileNbLocation;
	struct dirent *currentEntry;
	
	// Reserve one byte for the number of files
	fileNbLocation = ftell(packager.output);
	fputc(0, packager.output);
	
	currentEntry = readdir(root);
	if(currentEntry) {	
		writeModelRecursive(root, rootName, rootName, currentEntry);
		fseek(packager.output, fileNbLocation, SEEK_SET);
		fputc(packager.fileNb, packager.output);
	} else {
		printf("Error: %s is empty!\n", rootName);
		exit(1);
	}
}

void printBytes(FILE *in, int l) {
	for (; l > 0; l--) {
		printf("%c", fgetc(in));
	}
}

void printVersion(FILE *in) {
	versionData holder;
	holder.major = fgetc(in);
	holder.minor = fgetc(in);
	printf("%d.%d.%d", holder.major, holder.minor, fgetc(in));
}

void printMetadata(FILE *inputPackage) {
	int i, j, nbMeta;
	int key, len;
	// Used to print package dependencies
	int pkgnb, nlen;
	versionData depVersion;
	// Generic package testing
	char magic[5] = { 0 };
	
	for (i = 0; i < 4; i++) {
		magic[i] = fgetc(inputPackage);
	}
	
	if (strcmp(magic, "KPKG")) {
		printf("Provided file is not a valid KnightOS package !\n");
	} else if (fgetc(inputPackage) != KPKG_FORMAT_VERSION) {
		printf("This file uses an incompatible version of the KnightOS package format specification.\nConsider updating either kpack or the package itself.\n");
	} else {
		nbMeta = fgetc(inputPackage);
		
		for (i = 0; i < nbMeta; i++)
		{
			key = fgetc(inputPackage);
			len = fgetc(inputPackage);
			
			switch(key) {
				case KEY_PKG_NAME:
					printf("name=");
					printBytes(inputPackage, len);
					printf("\n");
					break;
				case KEY_PKG_REPO:
					printf("repo=");
					printBytes(inputPackage, len);
					printf("\n");
					break;
				case KEY_PKG_DESCRIPTION:
					printf("description=");
					printBytes(inputPackage, len);
					printf("\n");
					break;
				case KEY_PKG_DEPS:
					pkgnb = fgetc(inputPackage);
					printf("dependencies=");
					for (j = 0; j < pkgnb; j++) {
						depVersion.major = fgetc(inputPackage);
						depVersion.minor = fgetc(inputPackage);
						depVersion.patch = fgetc(inputPackage);
						nlen = fgetc(inputPackage);
						printBytes(inputPackage, nlen);
						if (depVersion.major != 0 || depVersion.minor != 0 || depVersion.patch != 0) {
							if (j == pkgnb - 1) {
								printf(":%d.%d.%d", depVersion.major, depVersion.minor, depVersion.patch);
							} else {
								printf(":%d.%d.%d ", depVersion.major, depVersion.minor, depVersion.patch);
							}
						} else {
							if (j != pkgnb - 1) {
								printf(" ");
							}
						}
					}
					break;
				case KEY_PKG_VERSION:
					printf("version=");
					printVersion(inputPackage);
					printf("\n");
					break;
				case KEY_PKG_AUTHOR:
					printf("author=");
					printBytes(inputPackage, len);
					printf("\n");
					break;
				case KEY_PKG_MAINTAINER:
					printf("maintainer=");
					printBytes(inputPackage, len);
					printf("\n");
					break;
				case KEY_PKG_COPYRIGHT:
					printf("copyright=");
					printBytes(inputPackage, len);
					printf("\n");
					break;
				case KEY_INFO_URL:
					printf("infourl=");
					printBytes(inputPackage, len);
					printf("\n");
					break;
				default:
					printf("Unknown meta field encountered: 0x%02X\n", key);
					break;
			}
		}
	}
}
