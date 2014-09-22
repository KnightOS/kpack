#include "common.h"
#include "packager.h"

void displayUsage() {
	printf(
		"KnightOS package manager - packs or unpacks a KnightOS package from or into a directory model.\n"
		"\n"
		"Usage: kpack [-c|--config configFile] [-e|--extract] [-s|--sum crc16|sha1|md5|none] [-x|--compressor pucrunch|rle|none] _package_ _model_\n"
		"See `man 1 kpack` for details.\n"
		"\n"
		"Options:\n"
		"\t-c|--config\n"
		"\t\tSpecifies an alternate config file.\n"
		"\t-e|--extract\n"
		"\t\tExtracts a package instead of creating one.\n"
		"\t-s|--sum\n"
		"\t\tSpecifies a checksum algorithm to use, among 'crc16', 'sha1', 'md5' and 'none', defaulting to 'crc16'\n"
		"\t-x|--compressor\n"
		"\t\tSpecifies the compressor to use among 'pucrunch', 'rle' and 'none', defaulting to 'pucrunch'\n"
	);
}

int main(int argc, char **argv) {
	FILE *output;
	initRuntime();
	
	if (parse_args(argc, argv)) {
		printf("Aborting operation.\n");
		return 1;
	}
	
	printf("Argument parsing successful.\n");
	
	// Pack things
	if (packager.pack) {
		if (parse_metadata()) {
			printf("Aborting operation.\n");
			if (packager.pkgname) {
				free(packager.pkgname);
			}
			if (packager.repo) {
				free(packager.repo);
			}
			fclose(packager.config);
			return 1;
		}
		
		printf("Metadata parsing successful.\nPackaging into file '%s'\n", packager.filename);
		
		output = fopen(packager.filename, "wb");
		
		// See doc/package_format for information on package format
		fputs("KPKG", output);
		fputc(KPKG_FORMAT_VERSION, output);
		
		// Write metadata
		fputc(packager.mdlen, output);
		// Package name
		fputc(KEY_PKG_NAME, output);
		fputc(strlen(packager.pkgname), output);
		fputs(packager.pkgname, output);
		// Package repo
		fputc(KEY_PKG_REPO, output);
		fputc(strlen(packager.repo), output);
		fputs(packager.repo, output);
		// Package version
		fputc(KEY_PKG_VERSION, output);
		fputc(3, output);
		fputc(packager.version.major, output);
		fputc(packager.version.minor, output);
		fputc(packager.version.patch, output);
		
		// Write files
		//
		// TODO
		//
		
		free(packager.pkgname);
		free(packager.repo);
		
		fclose(output);
		
		fclose(packager.config);
		
		printf("Packing done !\n");
	} else {
		printf("Unpacking not supported yet.\n");
	}
	
	return 0;
}
