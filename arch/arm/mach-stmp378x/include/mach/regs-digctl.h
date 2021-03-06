/*
 * stmp378x: DIGCTL register definitions
 *
 * Copyright (c) 2008 Freescale Semiconductor
 * Copyright 2008 Embedded Alley Solutions, Inc All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */
#define REGS_DIGCTL_BASE	(STMP3XXX_REGS_BASE + 0x1C000)
#define REGS_DIGCTL_PHYS	0x8001C000
#define REGS_DIGCTL_SIZE	0x2000

#define HW_DIGCTL_CTRL		0x0
#define BM_DIGCTL_CTRL_USB_CLKGATE	0x00000004

#define HW_DIGCTL_ARMCACHE	0x2B0
#define BM_DIGCTL_ARMCACHE_ITAG_SS	0x00000003
#define BP_DIGCTL_ARMCACHE_ITAG_SS	0
#define BM_DIGCTL_ARMCACHE_DTAG_SS	0x00000030
#define BP_DIGCTL_ARMCACHE_DTAG_SS	4
#define BM_DIGCTL_ARMCACHE_CACHE_SS	0x00000300
#define BP_DIGCTL_ARMCACHE_CACHE_SS	8
#define BM_DIGCTL_ARMCACHE_DRTY_SS	0x00003000
#define BP_DIGCTL_ARMCACHE_DRTY_SS	12
#define BM_DIGCTL_ARMCACHE_VALID_SS	0x00030000
#define BP_DIGCTL_ARMCACHE_VALID_SS	16
