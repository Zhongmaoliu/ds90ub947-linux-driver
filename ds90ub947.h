 /*
 * ******************************************************************************
 *                   Advanced development department
 *(c)Copyright 2018, Hangzhou Autotronics Electronics Co.Ltd. All Rights Reserved
 *                     
 * File Name        : ds90ub947.h
 * Description      : Driver 0f TI ds90ub947 serializer for Imx.6
 * Authors          : Allen.Liu
 * E-mail           : zmliujxgz@gmail.com
 * Create Date      : 2018-10-19
 * Modifications    : Allen.Liu
 * ******************************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
  * ******************************************************************************
 */
 
#ifndef _DS90UB947_H_
#define _DS90UB947_H_

#include <linux/spinlock.h>
#include <linux/kfifo.h>
#include <linux/interrupt.h>
#include <linux/poll.h>
#include <linux/i2c.h>

#define CHIP_DEVICE_NAME     "ds90ub947"
#define CHIP_ID_REG          0x00
#define DS90UB947_DEVICE_ID  0x1a

struct ds90ub947 {
	struct i2c_client	*client;
	unsigned char		i2c_addr;
		int					irq;
};

struct ds90ub947 ds90ub947s;

static struct miscdevice ds90ub947_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "ds90ub947_dev",
};

#endif
