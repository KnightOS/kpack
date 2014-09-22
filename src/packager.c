#include "common.h"

void initRuntime()
{
	packager.config = NULL;
	packager.configName = NULL;
	packager.pack = true;
	packager.rootName = NULL;
	packager.name = NULL;
	packager.version = 0;
	packager.mdlen = 0;
	packager.md = NULL;
	packager.compressionType = COMPRESSION_PUCRUNCH;
	packager.sumType = SUM_CRC16;
	packager.flen = 0;
	packager.files = NULL;
}

#define match_option(short, opt) (strcmp(opt, argv[i]) == 0 || strcmp(short, argv[i]) == 0)
int parse_args(int argc, char *argv[])
{
	int i;
	argc--;
	argv++;
	if(!argc)
	{
		printf("Invalid usage.\n\n");
		displayUsage();
		return 1;
	}
	for(i = 0; i < argc; i++)
	{
		if(match_option("-c", "--config"))
		{
			i++;
			packager.configName = argv[i];
		}
		else if(match_option("-e", "--extract"))
		{
			packager.pack = false;
		}
		else if(match_option("-s", "--sum"))
		{
			i++;
			if(strcmp(argv[i], "crc16") == 0)
				packager.sumType = SUM_CRC16;
			else if(strcmp(argv[i], "sha1") == 0)
				packager.sumType = SUM_SHA1;
			else if(strcmp(argv[i], "md5") == 0)
				packager.sumType = SUM_MD5;
			else if(strcmp(argv[i], "none") == 0)
				packager.sumType = SUM_NONE;
			else
			{
				printf("Invalid checksum type. Valid choices are 'crc16', 'sha1', 'md5' or 'none'.\n");
				return 1;
			}
		}
		else if(match_option("-x", "--compressor"))
		{
			i++;
			if(strcmp(argv[i], "pucrunch") == 0)
				packager.compressionType = COMPRESSION_PUCRUNCH;
			else if(strcmp(argv[i], "rle") == 0)
				packager.compressionType = COMPRESSION_RLE;
			else if(strcmp(argv[i], "none") == 0)
				packager.compressionType = COMPRESSION_NONE;
			else
			{
				printf("Invalid compression type. Valid choices are 'pucrunch', 'rle' or 'none'.\n");
				return 1;
			}
		}
		else
		{
			if(!packager.name)
				// That's the package's name
				packager.name = argv[i];
			else if(!packager.rootName)
				// That's the model
				packager.rootName = argv[i];
			else
			{
				// Hum ...
				printf("Unexpected extra argument.\n");
				return 1;
			}
		}
	}
	
	if(!packager.name)
	{
		printf("Expected a package name, none found.\n");
		return 1;
	}
	if(!packager.rootName)
	{
		printf("Expected a model name, none found.\n");
	}
	
	if(!packager.configName)
		packager.configName = "package.config";
		
	packager.config = fopen(packager.configName, "r");
	
	if(!packager.config)
	{
		printf("Config file '%s' not found.\n", packager.configName);
		return 1;
	}
	
	return 0;
}
