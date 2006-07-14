/*-
 * Copyright (c) 1998 Nicolas Souchu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 *
 */
#ifndef __IIC_H
#define __IIC_H

#include <sys/ioccom.h>

/* Designed to be compatible with linux's struct i2c_msg */
struct iic_msg
{
	uint16_t	slave;
	uint16_t	flags;
#define IIC_M_RD	0x0001	/* read vs write */
	uint16_t	len;	/* msg legnth */
	uint8_t *	buf;
};

struct iiccmd {
	u_char slave;
	int count;
	int last;
	char *buf;
};

struct iic_rdwr_data {
	struct iic_msg *msgs;
	uint32_t nmsgs;
};

#define I2CSTART	_IOW('i', 1, struct iiccmd)	/* start condition */
#define I2CSTOP		_IO('i', 2)			/* stop condition */
#define I2CRSTCARD	_IOW('i', 3, struct iiccmd)	/* reset the card */
#define I2CWRITE	_IOW('i', 4, struct iiccmd)	/* send data */
#define I2CREAD		_IOW('i', 5, struct iiccmd)	/* receive data */
#define I2CRDWR		_IOW('i', 6, struct iic_rdwr_data)	/* General read/write interface */

#endif
