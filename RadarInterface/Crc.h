unsigned short GetCrc(unsigned char *Buf, unsigned short lastcrc, unsigned long count);
unsigned short GetNextCrc(unsigned char ch,unsigned short lastcrc);
void AppendCrc(unsigned char *crcData, unsigned short lastcrc, unsigned long length);
bool CheckCrc(unsigned char *crcData, unsigned short lastcrc, unsigned long length);

