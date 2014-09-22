#include "common.h"
#include "packager.h"

void displayUsage()
{
	printf(
		"KnightOS package manager\n"
		"========================\n"
		"Usage:\n\n"
		"kpack [-c|--config configFile] [-e|--extract] [-s|--sum crc16|sha1|md5|none] [-x|--compressor pucrunch|rle|none] _package_ _model_\n"
		"\n\n"
		"Packs or unpacks a KnightOS project from or into a directory model.\n"
		"========================\n"
		"The following options are available:\n\n"
		"  -c|--config     : specifies an alternate config file.\n"
		"  -e|--extract    : extracts a package instead of creating one.\n"
		"  -s|--sum        : specifies a checksum algorithm to use, among 'crc16', 'sha1', 'md5' and 'none', defaulting to 'crc16'\n"
		"  -x|--compressor : specifies the compressor to use among 'pucrunch', 'rle' and 'none', defaulting to 'none'\n"
	);
}

int main(int argc, char *argv[])
{
	initRuntime();
	if(parse_args(argc, argv))
	{
		printf("Aborting operation.\n");
		return 1;
	}
	
	// TODO
	// Well, everything
	
	fclose(packager.config);
	
	return 0;
}
