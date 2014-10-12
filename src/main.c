#include "common.h"
#include "packager.h"
#include "unpack.h"

void displayUsage() {
	printf(
		"KnightOS package manager - Manipulates KnightOS software packages\n"
		"\n"
		"Usage: kpack [options] package model\n"
		"See `man 1 kpack` for details.\n"
		"\n"
		"Options:\n"
		"\t-c, --config\n"
		"\t\tSpecifies an alternate config file.\n"
		"\t-e, --extract\n"
		"\t\tExtracts a package instead of creating one.\n"
		"\t-h, --help\n"
		"\t\tDisplays this page.\n"
		"\t-i, --info\n"
		"\t\tPrints information about a given package.\n"
		"\t-s, --stubs\n"
		"\t\tExtracts package stubs when using -e. See man page for details.\n"
		"\t-k, --sum\n"
		"\t\tSpecifies a checksum algorithm to use. Valid options are:\n"
		"\t\tcrc16, sha1, md5, none (default: none)\n"
		"\t-x, --compressor\n"
		"\t\tSpecifies the compression algorithm to use. Valid options are:\n"
		"\t\tpucrunch, rle, none (default: none)'\n"
	);
}

void freeMetadata() {
	if(packager.pkgname) {
		free(packager.pkgname);
	}
	if(packager.repo) {
		free(packager.repo);
	}
	if(packager.description) {
		free(packager.description);
	}
	if(packager.author) {
		free(packager.author);
	}
	if(packager.maintainer) {
		free(packager.maintainer);
	}
	if(packager.copyright) {
		free(packager.copyright);
	}
	if(packager.infourl) {
		free(packager.infourl);
	}
}

int main(int argc, char **argv) {
	int argParsing;
	// File parsing
	DIR *rootDir;
	// For printing metadata
	FILE *inputPackage;
	initRuntime();
	
	argParsing = parse_args(argc, argv);
	
	if (argParsing > 0) {
		return 0;
	} else if (argParsing < 0) {
		printf("Aborting operation.\n");
		return 1;
	}
	
	
	// We're packing
	if (packager.pack) {
		if (parse_metadata()) {
			printf("Aborting operation.\n");
			freeMetadata();
			fclose(packager.config);
			return 1;
		}
		
		printf("Packaging %s/%s:%d.%d.%d as %s...\n", packager.repo, packager.pkgname,
				packager.version.major, packager.version.minor, packager.version.patch, packager.filename);
		
		packager.output = fopen(packager.filename, "wb");
		if (!packager.output) {
			printf("Error: unable to open %s for writing.\n", packager.filename);
			freeMetadata();
			return 1;
		}
		
		// See doc/package_format for information on package format
		fputs("KPKG", packager.output);
		fputc(KPKG_FORMAT_VERSION, packager.output);
		
		// Write metadata
		fputc(packager.mdlen, packager.output);
		// Package name
		fputc(KEY_PKG_NAME, packager.output);
		fputc(strlen(packager.pkgname), packager.output);
		fputs(packager.pkgname, packager.output);
		// Package repo
		fputc(KEY_PKG_REPO, packager.output);
		fputc(strlen(packager.repo), packager.output);
		fputs(packager.repo, packager.output);
		// Package version
		fputc(KEY_PKG_VERSION, packager.output);
		fputc(3, packager.output);
		fputc(packager.version.major, packager.output);
		fputc(packager.version.minor, packager.output);
		fputc(packager.version.patch, packager.output);
		// Then write all the optional metadata fields by iterating through them
		// Package description
		if(packager.description) {
			fputc(KEY_PKG_DESCRIPTION, packager.output);
			fputc(strlen(packager.description), packager.output);
			fputs(packager.description, packager.output);
			free(packager.description);
		}
		// Package's author
		if(packager.author) {
			fputc(KEY_PKG_AUTHOR, packager.output);
			fputc(strlen(packager.author), packager.output);
			fputs(packager.author, packager.output);
			free(packager.author);
		}
		// Package's maintainer
		if(packager.maintainer) {
			fputc(KEY_PKG_MAINTAINER, packager.output);
			fputc(strlen(packager.maintainer), packager.output);
			fputs(packager.maintainer, packager.output);
			free(packager.maintainer);
		}
		// Package's copyright
		if(packager.copyright) {
			fputc(KEY_PKG_COPYRIGHT, packager.output);
			fputc(strlen(packager.copyright), packager.output);
			fputs(packager.copyright, packager.output);
			free(packager.copyright);
		}
		// Package's info URL
		if(packager.infourl) {
			fputc(KEY_INFO_URL, packager.output);
			fputc(strlen(packager.infourl), packager.output);
			fputs(packager.infourl, packager.output);
			free(packager.infourl);
		}
		// Package dependencies
		if(packager.dependencies_len != 0) {
			fputc(KEY_PKG_DEPS, packager.output);
			int len = 1;
			int i;
			for (i = 0; i < packager.dependencies_len; ++i) {
				len += 4;
				len += strlen(packager.dependencies[i]->name);
			}
			fputc(len, packager.output);
			fputc(packager.dependencies_len, packager.output);
			for (i = 0; i < packager.dependencies_len; ++i) {
				fputc(packager.dependencies[i]->version.major, packager.output);
				fputc(packager.dependencies[i]->version.minor, packager.output);
				fputc(packager.dependencies[i]->version.patch, packager.output);
				fputc(strlen(packager.dependencies[i]->name), packager.output);
				fputs(packager.dependencies[i]->name, packager.output);
			}
		}
		
		// Write files
		rootDir = opendir(packager.rootName);
		if(!rootDir) {
			printf("Couldn't open directory %s.\nAborting operation.\n", packager.rootName);
		} else {
			writeModel(rootDir, packager.rootName);
			closedir(rootDir);
		}
		
		fclose(packager.output);
		fclose(packager.config);
		printf("Successfully wrote %s/%s:%d.%d.%d to %s.\n", packager.repo, packager.pkgname,
				packager.version.major, packager.version.minor, packager.version.patch, packager.filename);
		free(packager.pkgname);
		free(packager.repo);
	} else {
		// We're not packing, then what are we doing ?
		if (packager.printMeta)
		{
			// We're printing metadata from a package
			inputPackage = fopen(packager.filename, "rb");
			if (!inputPackage) {
				printf("Couldn't find %s. Aborting operation.\n", packager.filename);
			} else {
				printMetadata(inputPackage);
				fclose(inputPackage);
				printf("\n");
			}
		} else {
			// We're unpacking
			inputPackage = fopen(packager.filename, "rb");
			if (!inputPackage) {
				printf("Unable to open %s for reading.\n", packager.filename);
			} else {
				if (packager.rootName[strlen(packager.rootName) - 1] == '/') {
					packager.rootName[strlen(packager.rootName) - 1] = '\0';
				}
				unpack(inputPackage, packager.rootName, packager.stub);
				fclose(inputPackage);
				printf("Extracted %s.\n", packager.filename);
			}
		}
	}
	
	return 0;
}
