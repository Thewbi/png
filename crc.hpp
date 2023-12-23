#ifndef __CRC__
#define __CRC__

void make_crc_table(void);

unsigned long update_crc(unsigned long crc, unsigned char *buf, int len);

unsigned long crc(unsigned char *buf, int len);

#endif