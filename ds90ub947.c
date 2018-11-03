 /*
 * ******************************************************************************
 *                   Advanced development department
 *(c)Copyright 2018, Hangzhou Autotronics Electronics Co.Ltd. All Rights Reserved
 *                     
 * File Name        : ds90ub947.c
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

#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/freezer.h>
#include <linux/input.h>
#include <linux/jiffies.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <linux/workqueue.h>
#include <linux/hrtimer.h> 
#include <linux/time.h> 

#include "ds90ub947.h"

static int ds90ub947_client_init(struct ds90ub947 *ds90ub947)
{

	return 0;
}


static int ds90ub947_i2c_read(struct i2c_client *i2c,
				unsigned char reg_addr,
				unsigned short length, unsigned char *data)
{
	struct i2c_msg msgs[2];
	int err, retry = 3;
	if(!data)
		return -EINVAL;
	
	msgs[0].addr = i2c->addr;
	msgs[0].flags = 0;
	msgs[0].len = 1;
	msgs[0].buf = &reg_addr;
	
	msgs[1].addr = i2c->addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = length;
	msgs[1].buf = data;

	while((err = i2c_transfer(i2c->adapter, msgs, 2)) < 2)
	{
		if(retry-- <= 0)
		{
			pr_err("%s: transfer failed %d", __func__, err);
			return -EIO;
		}
	}
	return 0;
}


static int ds90ub947_i2c_single_write(struct i2c_client *i2c,
                         unsigned char reg_addr, unsigned char value)
{
    struct i2c_msg msgs[1];
    unsigned char data[2];
	int err, retry = 3;
    data[0] = reg_addr;
    data[1] = value;

    /* Write Message */
    msgs[0].addr = i2c->addr;
    msgs[0].flags = 0;
    msgs[0].buf = (unsigned char *) data;
    msgs[0].len = 2;

	while((err = i2c_transfer(i2c->adapter, msgs, 1)) < 1)
    {
    	if(retry-- <= 0)  {
			pr_err("%s: transfer failed %d", __func__, err);
			return -EIO;
		}
	}
	return 0;
}


/* I2C I/O function */
static int ds90ub947_read(struct ds90ub947 *ds90ub947,
				unsigned char reg_addr,
				unsigned short length, unsigned char *data)
{
	return ds90ub947_i2c_read(ds90ub947->client, reg_addr, length, data);
}				


static int ds90ub947_single_write(struct ds90ub947 *ds90ub947,
                         unsigned char reg_addr, unsigned char value)
{
	return ds90ub947_i2c_single_write(ds90ub947->client, reg_addr, value);
}


static int ds90ub947_chip_init(struct ds90ub947 *ds90ub947)
{
	int err;
	int reg_data;
	/* Configure Registers
	 *Note: This is also the hardware reset value for these registers.
	 */	
	//Chip id 
	reg_data = ds90ub947_read(ds90ub947, 0x4f, 1, 0x00);
	
	//OLDI_DUAL:Single-pixel mode,MAPSEL: OpenLDI Bit Mapping
	err = ds90ub947_single_write(ds90ub947, 0x4f, 0x40);
	if (err)
	{
		pr_err("%s: ds90ub947_single_write failed.", __func__);
		return err;
	}
	msleep(5);
	
	//AUTO_SS:Auto Sleep-State
	err = ds90ub947_single_write(ds90ub947, 0x01, 0x00);
	if (err)
	{
		pr_err("%s: ds90ub947_single_write failed.", __func__);
		return err;
	}
	msleep(5);
	
	//REPEATER:Disable repeater mode
	err = ds90ub947_single_write(ds90ub947, 0xc2, 0x98);
	if (err)
	{
		pr_err("%s: ds90ub947_single_write failed.", __func__);
		return err;
	}
	msleep(5);
	
	//COAX:Enable FPD-Link III for coaxial cabling
	err = ds90ub947_single_write(ds90ub947, 0x5b, 0xa0);
	if (err)
	{
		pr_err("%s: ds90ub947_single_write failed.", __func__);
		return err;
	}
	msleep(5);
	
	return 0;
}

static int ds90ub947_check_device(struct i2c_client *client)
{
	unsigned char buffer[2];
	int err = 0;

	buffer[0] = CHIP_ID_REG;
	err = ds90ub947_i2c_read(client, CHIP_ID_REG, 1, buffer);	
	if (err < 0) {
		pr_err("%s: Can not read ds90ub947\n", __func__);
		return err;
	}
	buffer[0] = buffer[0] >> 1;
	if(buffer[0] == DS90UB947_DEVICE_ID)
		pr_info("%s: %s is mounted.\n", __func__, CHIP_DEVICE_NAME);
	else
	{
		pr_err("%s: unknow chip is mounted, read :0x%02x, expected DS90UB947_DEVICE_ID is 0x%02x\n", __func__, buffer[0], DS90UB947_DEVICE_ID);
		return -ENXIO;
	}

	return err;
}


int ds90ub947_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct ds90ub947 *ds90ub947;
	int err = 0;

	printk("%s start probing... I2C mode\n", CHIP_DEVICE_NAME);
	/* check i2c connection */
	// if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		// pr_info("%s: %s is mounted.\n", __func__, CHIP_DEVICE_NAME);
		// err = -ENODEV;
		// goto exit0;
	// }
	/* check device */
	err = ds90ub947_check_device(client);
	if (err)
		goto exit0;
	/* Allocate memory for driver data */
	ds90ub947 = kzalloc(sizeof(ds90ub947s), GFP_KERNEL);
	if (!ds90ub947) {
		pr_err("%s: memory allocation failed.\n", __func__);
		err = -ENOMEM;
		goto exit1;
	}
	/* set client data */
	ds90ub947->client = client;
	i2c_set_clientdata(client, ds90ub947);
	ds90ub947->irq = client->irq;
	/* register misc device for iam20680 */
	err = misc_register(&ds90ub947_dev);
	if (err) {
		pr_err("%s: ds90ub947 register failed", __func__);
		goto exit2;			// free device memory
	}

	ds90ub947_chip_init(ds90ub947);
	ds90ub947_client_init(ds90ub947);

	pr_debug("%s successfully probed.", __func__);
	return 0;
	
exit2:
	kfree(ds90ub947);	
exit1:
exit0:
	return err;
}


static int ds90ub947_remove(struct i2c_client *client)
{
	struct ds90ub947 *ds90ub947 = i2c_get_clientdata(client);
	
	misc_deregister(&ds90ub947_dev);
	
	kfree(ds90ub947);
	pr_debug("successfully removed.");
	return 0;
}

static const struct i2c_device_id ds90ub947_id[] = {
	{CHIP_DEVICE_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ds90ub947_id);

static struct i2c_driver ds90ub947_i2c_driver = {
	.probe		= ds90ub947_probe,
	.remove 	= ds90ub947_remove,
	.id_table	= ds90ub947_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "ds90ub947",
	},
};

static int __init ds90ub947_init(void)
{
	return i2c_add_driver(&ds90ub947_i2c_driver);
}

static void __exit ds90ub947_exit(void)
{
	i2c_del_driver(&ds90ub947_i2c_driver);
}


module_init(ds90ub947_init);
module_exit(ds90ub947_exit);

MODULE_AUTHOR("Hangzhou Autotronics Electronics Co.Ltd.");
MODULE_DESCRIPTION("DS90UB947 Serializer Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_ALIAS("947");
