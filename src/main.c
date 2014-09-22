#include "common.h"
#include "packager.h"

void displayUsage()
{
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
		"\t\tSpecifies the compressor to use among 'pucrunch', 'rle' and 'none', defaulting to 'none'\n"
	);
}

int main(int argc, char **argv)
{
	initRuntime();
	
	if(parse_args(argc, argv)) {
		printf("Aborting operation.\n");
		return 1;
	}
	
	// TODO
	// Well, everything
	
	fclose(packager.config);
	
	return 0;
}
