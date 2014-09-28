#include "common.h"

uint16_t calculateCRC16(FILE *in) {
	uint16_t crcVal = 0xffff;
	int b, c, backupSize = ftell(in);
	
	fseek(in, 0, SEEK_SET);
	
	while((c = fgetc(in)) != EOF) {
		for(b = 0; b < 8; b++, c <<= 1) {
			if((crcVal ^ (c >> 7)) & 0x8000) {
				crcVal = (crcVal << 1) ^ 0x68da;
			} else {
				crcVal <<= 1;
			}
		}
	}
	
	fseek(in, backupSize, SEEK_SET);
	return crcVal;
}
