#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <dirent.h>

#define PACK_UNCOMPRESSED 0
#define PACK_RLE 1
#define PACK_PUCRUNCH 2

#define SUM_NONE 0
#define SUM_CRC16 1
#define SUM_SHA1 2
#define SUM_MD5 3

struct {
	char *package_file;
	char *model_dir;
	char *config_file;
	int extract;
	uint8_t compression_algorithm;
	void (*write_compressed)(FILE *, char *);
	uint8_t checksum_algorithm;
	char* (*get_checksum)(char *);
} runtime;

void kpack_abort(char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fprintf(stderr, "\n");
	exit(1);
}

#define match(short, long) strcmp(short, argv[i]) == 0 || strcmp(long, argv[i]) == 0

void parse_arguments(int argc, char **argv) {
	runtime.package_file = NULL;
	runtime.model_dir = NULL;
	runtime.config_file = "pkgconfig.ini";
	runtime.extract = 0;
	runtime.compression_algorithm = PACK_PUCRUNCH;
	runtime.checksum_algorithm = SUM_CRC16;
	runtime.write_compressed = NULL;
	runtime.get_checksum = NULL;

	int i;
	for (i = 0; i < argc; ++i) {
		if (argv[i][0] == '-' && argv[i][1] != '\0') {
			if (match("-e", "--extract")) {
				runtime.extract = 1;
			} else if (match("-c", "--config")) {
				runtime.config_file = argv[++i];
			} else if (match("-x", "--compressor")) {
				// TODO
			} else if (match("-s", "--sum")) {
				// TODO
			}
		} else {
			// TODO
		}
	}
}

void parse_config() {
	FILE *f = fopen(runtime.config_file, "r");
	if (!f) {
		kpack_abort("Unable to open config file: %s", runtime.config_file);
	}
	char *line = malloc(128);
	int i = 0;
	int len = 128;
	while (!feof(f)) {
		char c = fgetc(f);
		if (c != '\n') {
			line[i++] = c;
			if (i == len) {
				line = realloc(line, len + 128);
				len += 128;
			}
		} else {
			// Parse this line
			line[i] = '\0';
			i = 0;
			if (strlen(line) == 0 || line[0] == '#') {
				continue;
			}
			// TODO: Do something with this line
		}
	}
	fclose(f);
}

int main(int argc, char **argv) {
	parse_arguments(argc, argv);
	parse_config();
	return 0;
}
