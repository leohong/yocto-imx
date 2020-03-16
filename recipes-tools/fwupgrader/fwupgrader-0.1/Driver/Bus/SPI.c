
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "types.h"
#include "SPI.h"

static pthread_mutex_t spi_mutex;
static int spifile = 0;

static uint8_t bits = 8;
static uint32_t speed = 500000;
static uint8_t mode = SPI_MODE_3;

// This has to be called from main task!
BOOL SPIOpen(void) {
	pthread_mutex_init(&spi_mutex, NULL);
	spifile=open("/dev/spidev32766.0", O_RDWR);
	if(spifile) {
		ioctl(spifile, SPI_IOC_WR_BITS_PER_WORD, &bits);
		ioctl(spifile, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
        ioctl(spifile, SPI_IOC_WR_MODE, &mode);
		return TRUE;
	}
	else return FALSE;
}

// This has to be called from main task!
void SPIClose(void) {
	if(spifile) {
		close(spifile);
		spifile = 0;
	}
}

BOOL SPITransferBuffer(UBYTE *TxBuf, UBYTE *RxBuf, int BufLen) {
	int ret;
	uint8_t result;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)TxBuf,
		.rx_buf = (unsigned long)RxBuf,
		.len = BufLen,
		.delay_usecs = 0,
		.speed_hz = speed,
		.bits_per_word = bits,
        .cs_change = 0,
	};

	pthread_mutex_lock(&spi_mutex);
	ret = ioctl(spifile, SPI_IOC_MESSAGE(1), &tr);
	pthread_mutex_unlock(&spi_mutex);

	if (ret < 1) return FALSE;
	return TRUE;
}

UBYTE SPITransferByte(UBYTE TxValue) {
	int ret;
	UBYTE RxValue;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)&TxValue,
		.rx_buf = (unsigned long)&RxValue,
		.len = 1,
		.delay_usecs = 0,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	pthread_mutex_lock(&spi_mutex);
	ret = ioctl(spifile, SPI_IOC_MESSAGE(1), &tr);
	pthread_mutex_unlock(&spi_mutex);

	return RxValue;
}


