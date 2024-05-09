/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *
 */

#ifndef __IMX8MP_FT20_H
#define __IMX8MP_FT20_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>

#define CONFIG_SYS_UBOOT_BASE	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#define BOOT_TARGET_DEVICES(func) \
	func(USB, usb, 0) \
	func(MMC, mmc, 1)

#include <config_distro_bootcmd.h>

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"image=Image\0" \
	"console=ttymxc3,115200 earlycon=ec_imx6q,0x30A60000,115200\0" \
	"fdt_addr_r=0x43000000\0"			\
	"fdt_addr=0x43000000\0" \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"boot_fdt=try\0" \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0" \
	"mmcpart=1\0" \
	"mmcroot=" CONFIG_MMCROOT " rootwait rw\0" \
	"mmcautodetect=yes\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"root=/dev/mmcblk${mmcdev}p${mmcroot} rootwait rw\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"if run loadfdt; then " \
			"booti ${loadaddr} - ${fdt_addr}; " \
		"else " \
			"echo WARN: Cannot load the DT; " \
		"fi;\0 " \

/* UART4 Console Config */
#define CFG_MXC_UART_BASE		UART4_BASE_ADDR
#define CFG_CONSOLE_DEV			"ttymxc3"

/* USDHC2 SD Boot Config */
#define CONFIG_SYS_MMC_ENV_DEV		1			/* USDHC2 */
#define CONFIG_MMCROOT			"/dev/mmcblk1p2"	/* USDHC2 */
#define CONFIG_SYS_FSL_USDHC_NUM	1

/* Link Definitions */
#define CFG_SYS_INIT_RAM_ADDR		0x40000000
#define CFG_SYS_INIT_RAM_SIZE		0x80000

/* 4GB LPDDR4 Config */
#define CFG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM			0x40000000
#define PHYS_SDRAM_SIZE			0x80000000      /* 2 GB */
#define PHYS_SDRAM_2			0xC0000000
#define PHYS_SDRAM_2_SIZE		0x80000000      /* 2 GB */

#endif /* __IMX8MP_FT20_H */
