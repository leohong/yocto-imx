#ifndef SPI_H
#define SPI_H

BOOL SPIOpen(void);
void SPIClose(void);
BOOL SPITransferBuffer(UBYTE *TxBuf, UBYTE *RxBuf, int BufLen);
UBYTE SPITransferByte(UBYTE TxValue);

#endif // SPI_H

