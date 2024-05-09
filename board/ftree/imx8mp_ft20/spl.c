// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2018-2019, 2021 NXP
 *
 */

#include <common.h>
#include <command.h>
#include <cpu_func.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <power/pmic.h>
#include <power/pca9450.h>
#include <asm/arch/clock.h>
#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <fsl_esdhc_imx.h>
#include <mmc.h>
#include <asm/arch/ddr.h>
#include "lpddr4_timing.h"


DECLARE_GLOBAL_DATA_PTR;


#define UART_PAD_CTRL		(PAD_CTL_DSE6 | PAD_CTL_FSEL1)
#define WDOG_PAD_CTRL		(PAD_CTL_DSE6 | PAD_CTL_ODE | PAD_CTL_PUE | PAD_CTL_PE)

#define USDHC_PAD_CTRL		(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE | PAD_CTL_FSEL2)
#define USDHC_GPIO_PAD_CTRL 	(PAD_CTL_PUE  | PAD_CTL_DSE1)


int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	return BOOT_DEVICE_BOOTROM;
}


/* FT20 UART_4 is console/debug for A53 */
static iomux_v3_cfg_t const uart_pads[] = {
	MX8MP_PAD_NAND_CLE__UART4_DCE_RX	| MUX_PAD_CTRL(UART_PAD_CTRL),
	MX8MP_PAD_NAND_DATA01__UART4_DCE_TX	| MUX_PAD_CTRL(UART_PAD_CTRL),
};


static iomux_v3_cfg_t const wdog_pads[] = {
	MX8MP_PAD_GPIO1_IO02__WDOG1_WDOG_B	| MUX_PAD_CTRL(WDOG_PAD_CTRL),
};


iomux_v3_cfg_t const usdhc2_pads[] = {
        MX8MP_PAD_SD2_CLK__USDHC2_CLK		| MUX_PAD_CTRL(USDHC_PAD_CTRL),
        MX8MP_PAD_SD2_CMD__USDHC2_CMD		| MUX_PAD_CTRL(USDHC_PAD_CTRL),
        MX8MP_PAD_SD2_DATA0__USDHC2_DATA0	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
        MX8MP_PAD_SD2_DATA1__USDHC2_DATA1	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
        MX8MP_PAD_SD2_DATA2__USDHC2_DATA2	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
        MX8MP_PAD_SD2_DATA3__USDHC2_DATA3	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
        MX8MP_PAD_SD2_CD_B__USDHC2_CD_B		| MUX_PAD_CTRL(USDHC_GPIO_PAD_CTRL),
};


void ft20_imx8mp_early_init_f(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;
	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));

	set_wdog_reset(wdog);

	imx_iomux_v3_setup_multiple_pads(uart_pads,   ARRAY_SIZE(uart_pads));
	imx_iomux_v3_setup_multiple_pads(usdhc2_pads, ARRAY_SIZE(uart_pads));

	init_uart_clk(3);	//Titus: uart4
}


void spl_dram_init(void)
{
	struct dram_timing_info *dram_timing;

	int size = 4096;

	/*
	* - 4GiB Micron MT53D1024M32D4DT 2-ch dual-die per channel
	*/

	dram_timing = &dram_timing_4gb_dual_die;

	printf("DRAM : MT53D1024M32D4DT");
	printf(" %dMiB", size);
	printf(" %dMT/s %dMHz\n", dram_timing->fsp_msg[0].drate, dram_timing->fsp_msg[0].drate / 2);

	ddr_init(dram_timing);
}


void spl_board_init(void)
{
	/*
	 * Set GIC clock to 500 MHz for OD VDD_SOC. Kernel driver does not
	 * allow to change it. Should set the clock after PMIC setting done.
	 * Default is 400 MHz (system_pll1_800m with div = 2) set by ROM for
	 * ND VDD_SOC.
	 */
	clock_enable(CCGR_GIC, 0);
	clock_set_target_val(GIC_CLK_ROOT, CLK_ROOT_ON | CLK_ROOT_SOURCE_SEL(5));
	clock_enable(CCGR_GIC, 1);
}


int power_init_board(void)
{
	struct udevice *dev;
	int ret;

	ret = pmic_get("pmic@25", &dev);
	if (ret == -ENODEV) {
		puts("Failed to get PMIC\n");
		return 0;
	}
	if (ret != 0)
		return ret;

	/* BUCKxOUT_DVS0/1 control BUCK123 output. */
	pmic_reg_write(dev, PCA9450_BUCK123_DVS, 0x29);

	/* Increase VDD_SOC to typical value 0.95V before first DRAM access. */
	if (IS_ENABLED(CONFIG_IMX8M_VDD_SOC_850MV))
		/* Set DVS0 to 0.85V for special case. */
		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x14);
	else
		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x1c);

	/* Set DVS1 to 0.85v for suspend. */
	pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS1, 0x14);

	/*
	 * Enable DVS control through PMIC_STBY_REQ and
	 * set B1_ENMODE=1 (ON by PMIC_ON_REQ=H).
	 */
	pmic_reg_write(dev, PCA9450_BUCK1CTRL, 0x59);

	/* Kernel uses OD/OD frequency for SoC. */

	/* To avoid timing risk from SoC to ARM, increase VDD_ARM to OD voltage 0.95V */
	pmic_reg_write(dev, PCA9450_BUCK2OUT_DVS0, 0x1c);

	/* Set LDO4 and CONFIG2 to enable the I2C level translator. */
	pmic_reg_write(dev, PCA9450_LDO4CTRL, 0x59);
	pmic_reg_write(dev, PCA9450_CONFIG2, 0x1);

	return 0;
}


void board_init_f(ulong dummy)
{
	struct udevice *dev;
	int ret;

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	arch_cpu_init();

	ft20_imx8mp_early_init_f();

	timer_init();

	preloader_console_init();

	ret = spl_early_init();
	if (ret) {
		debug("spl_init() failed: %d\n", ret);
		hang();
	}

	ret = uclass_get_device_by_name(UCLASS_CLK,
					"clock-controller@30380000",
					&dev);
	if (ret < 0) {
		printf("Failed to find clock node. Check device tree\n");
		hang();
	}

	enable_tzc380();

	power_init_board();

	/* LPDDR4 initialization */
	spl_dram_init();

	board_init_r(NULL, 0);
}
