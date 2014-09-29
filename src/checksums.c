#include "common.h"
#include <string.h>

uint16_t calculateCRC16(FILE *in) {
	uint16_t crcVal = 0xffff;
	int b, c, backupSize = ftell(in);
    
	fseek(in, 0, SEEK_SET);
	
	while ((c = fgetc(in)) != EOF) {
		for(b = 0; b < 8; b++) {
			if ((crcVal >> 15) ^ (c >> 7)) {
				crcVal = (crcVal << 1) ^ 0x68da;
			} else {
				crcVal <<= 1;
			}
            c <<= 1;
			c &= 0xff;
		}
	}
	
	fseek(in, backupSize, SEEK_SET);
	return crcVal;
}
