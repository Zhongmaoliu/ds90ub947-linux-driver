Kernel:linux 3.10 and above

Hardware:ARM

Step:

1.copy ds90ub947.c kernel/driver/misc

2.copy ds90ub947.h kernel/include/linux/misc

3.modify kconfig and Makefile:

cd /driver/misc/konfig,add the list

config DS90UB947
	tristate "TI DS90UB947 FPD-Link de-/serializer chip"
	default n
	depends on I2C
	help
	  if you say yes here you get support for the DS90UB947 series of
	  chips.
	  
cd /driver/misc/Makefile,add the list

obj-$(CONFIG_DS90UB947)	+= ds90ub947.o

4.make menuconfig

select * in the [] of ds90ub947

5.dtbs

&i2c1{
	clock-frequency = <400000>;
	
	ds90ub947@1a {
		compatible = "ti,ds90ub947";
		reg = <0x1a>;
		
	};

};

Help:if you have trouble, you can send email to <zmliujxgz@gmail.com>.
