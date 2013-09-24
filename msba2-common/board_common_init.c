/*
 * board_common_init.c - common initialization of the msba2-based boards.
 *
 * Copyright (C) 2013 Zakaria Kasmi <zkasmi@inf.fu-berlin.de>
 *
 * This source code is licensed under the LGPLv2 license,
 * See the file LICENSE for more details.
 */

/**
 * @ingroup msba2
 * @{
 */

/**
 * @file
 * @brief       MSB-A2 based board initialization
 *
 * @author      Freie Universität Berlin, Computer Systems & Telematics
 * @author      Heiko Will
 * @author      Kaspar Schleiser
 * @author      Michael Baar <baar@inf.fu-berlin.de>
 * @author      Zakaria Kasmi <zkasmi@inf.fu-berlin.de>
 * @note        $Id: board_common_init.c 3857 2013-09-24 18:55:25 kasmi $
 */
#include <board.h>
#include <lpc23xx.h>
#include <VIC.h>
#include <cpu.h>
#include <config.h>
#include <string.h>
#include <flashrom.h>

#define PCRTC         BIT9
#define CL_CPU_DIV    4

/*---------------------------------------------------------------------------*/
/**
 * @brief   Enabling MAM and setting number of clocks used for Flash memory
 *          fetch
 * @internal
 */
static void init_mam(void)
{
    MAMCR  = 0x0000;
    MAMTIM = 0x0003;
    MAMCR  = 0x0002;
}

static inline void pllfeed(void)
{
    PLLFEED = 0xAA;
    PLLFEED = 0x55;
}

void init_clks2(void)
{
    // Wait for the PLL to lock to set frequency
    while (!(PLLSTAT & BIT26));

    // Connect the PLL as the clock source
    PLLCON = 0x0003;
    pllfeed();

    /* Check connect bit status */
    while (!(PLLSTAT & BIT25));
}

void bl_init_clks(void)
{
    PCONP = PCRTC;   // switch off everything except RTC
    init_clks1();
    init_clks2();
    init_mam();
}

