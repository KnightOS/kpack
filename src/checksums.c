#include "common.h"

uint16_t calculateCRC16(FILE *in) {
	uint16_t crcVal = 0xffff;
	int c, backupSize = ftell(in);
	
	fseek(in, 0, SEEK_SET);
	
	while ((c = fgetc(in)) != EOF) {
		if ((crcVal ^ ((uint8_t)c << 8)) & 0x8000) {
			crcVal = (crcVal << 1) ^ 0x1021; // x^12 + x^5 + 1
		} else {
			crcVal <<= 1;
		}
	}
	
	fseek(in, backupSize, SEEK_SET);
	return crcVal;
}
