// SPDX-License-Identifier: GPL-2.0+
/*
 *
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <dm.h>
#include <dt-bindings/clock/imx8mp-clock.h>
#include <env.h>
#include <env_internal.h>
#include <i2c_eeprom.h>
#include <linux/bitfield.h>
#include <malloc.h>
#include <net.h>
#include <miiphy.h>

#include "lpddr4_timing.h"

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

int board_late_init(void)
{
	return 0;
}

int fdtdec_board_setup(const void *fdt_blob)
{
	return 0;
}