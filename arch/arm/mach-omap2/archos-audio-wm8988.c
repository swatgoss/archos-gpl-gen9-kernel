#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/err.h>

#include <mach/gpio.h>
#include <plat/archos-audio-wm8988.h>
#include <plat/archos-gpio.h>
#include <asm/mach-types.h>
#include <plat/board.h>
#include <mach/board-archos.h>
#include <linux/delay.h>

#include "clock.h"
#include "mux.h"

static struct archos_audio_conf audio_gpio;

extern int archos_get_highcharge_level(void);

/* WORKAROUND: There is no way to keep CM_96M_FCLK (the sys_clkout2 source)
 * alive when every module which depends on it is idle, So we have to keep
 * the McBSP1 fclk ON.
 */
int use_mcbsp1_fclk = 1;

static struct platform_device audio_soc_device = {
	.name		= "archos-wm8988-asoc",
	.id		= -1,
	.dev            = {
		.platform_data = NULL,
	},
};

static void _set_ampli(int onoff)
{
	if (audio_gpio.spdif < 0) {
		printk(KERN_ERR "No SPDIF in this device !\n");
		return;
	}

	if (onoff)
		gpio_set_value(audio_gpio.spdif, 0);
	else
		gpio_set_value(audio_gpio.spdif, 1);
}

static void _set_vamp(int onoff)
{
#if 0
	if (audio_gpio.vamp_vbat < 0 || audio_gpio.vamp_dc < 0) {
		printk(KERN_ERR "No Vamp config in this device !\n");
		return;
	}
	
	if (onoff) {
		if (archos_get_highcharge_level()) {
			gpio_set_value(audio_gpio.vamp_dc, 1);
			gpio_set_value(audio_gpio.vamp_vbat, 0);
		} else {
			gpio_set_value(audio_gpio.vamp_vbat, 1);
			gpio_set_value(audio_gpio.vamp_dc, 0);
		}
		msleep(300);	// fix me
	} else {
		msleep(10);
		gpio_set_value(audio_gpio.vamp_vbat, 0);
		gpio_set_value(audio_gpio.vamp_dc, 0);
	}
#endif
}

static void _set_hp(int onoff)
{
	if (audio_gpio.hp_on < 0) {
		printk(KERN_ERR "No Speaker in this device !\n");
		return;
	}

	if (onoff) {
		_set_vamp(onoff);
		gpio_set_value(audio_gpio.hp_on, 1);
	} else {
		gpio_set_value(audio_gpio.hp_on, 0);
		_set_vamp(onoff);
	}
}

static int _get_headphone_plugged(void)
{
	if (audio_gpio.headphone_plugged < 0) {
		printk(KERN_ERR "No Headphone detection in this device !\n");
		return -1;
	}

	return gpio_get_value(audio_gpio.headphone_plugged);
}

static int _get_headphone_irq(void)
{
	if (audio_gpio.headphone_plugged < 0) {
		printk(KERN_ERR "No Headphone detection in this device !\n");
		return -1;
	}

	return gpio_to_irq(audio_gpio.headphone_plugged);
}

static void enable_mcbs1_fclk(int en)
{
	struct clk *sys_clkout;

	sys_clkout = clk_get(NULL, "mcbsp1_fck");
	if (!IS_ERR(sys_clkout)) {
		if (en) {
			if (clk_enable(sys_clkout) != 0) {
				printk(KERN_ERR "failed to enable %s\n", "mcbsp1_fck");
			}
		} else {
			clk_disable(sys_clkout);
		}

		clk_put(sys_clkout);
	}
}

static void _sys_clkout_en(int en)
{
	struct clk *sys_clkout;

	sys_clkout = clk_get(NULL, "sys_clkout2");
	if (!IS_ERR(sys_clkout)) {
		if (en) {
			if (use_mcbsp1_fclk) {
				enable_mcbs1_fclk(1);
			}
			if (clk_enable(sys_clkout) != 0) {
				printk(KERN_ERR "failed to enable %s\n", "sys_clkout2");
			}
		} else {
			clk_disable(sys_clkout);
			if (use_mcbsp1_fclk) {
				enable_mcbs1_fclk(0);
			}
		}

		clk_put(sys_clkout);
	}
}

static int _get_clkout_rate(void)
{
	struct clk *sys_clkout;
	int rate;

	sys_clkout = clk_get(NULL, "sys_clkout2");
	if (IS_ERR(sys_clkout)) {
		printk(KERN_ERR "failed to get %s\n", "sys_clkout2");
	}
	rate = clk_get_rate(sys_clkout);
	clk_put(sys_clkout);

	return rate;
}

static void _suspend(void)
{
}

static void _resume(void)
{
}

static struct audio_wm8988_device_config audio_wm8988_device_io = {
	.set_spdif = &_set_ampli,
	.get_headphone_plugged =&_get_headphone_plugged,
	.get_headphone_irq =&_get_headphone_irq,
	.set_speaker_state = &_set_hp,
	.set_codec_master_clk_state = &_sys_clkout_en,
	.get_master_clock_rate = &_get_clkout_rate,
	.suspend = &_suspend,
	.resume = &_resume,
};

struct audio_wm8988_device_config *archos_audio_wm8988_get_io(void) {
		return &audio_wm8988_device_io;
}

int __init archos_audio_wm8988_init(void)
{
	const struct archos_audio_config *audio_cfg;
	struct clk *clkout2_src_ck;
	struct clk *sys_clkout2;
	struct clk *core_ck;

	pr_debug("Archos audio init (WM8988)\n");

	/* audio  */
	audio_cfg = omap_get_config( ARCHOS_TAG_AUDIO, struct archos_audio_config );
	if (audio_cfg == NULL) {
		printk(KERN_DEBUG "%s: no board configuration found\n", __FUNCTION__);
		return -ENODEV;
	}
	if ( hardware_rev >= audio_cfg->nrev ) {
		printk(KERN_DEBUG "%s: hardware_rev (%i) >= nrev (%i)\n",
			__FUNCTION__, hardware_rev, audio_cfg->nrev);
		return -ENODEV;
	}

	audio_gpio = audio_cfg->rev[hardware_rev];

	omap_mux_init_signal("sys_clkout2.sys_clkout2", OMAP_PIN_OUTPUT);

	core_ck = clk_get(NULL, "cm_96m_fck");
	if (IS_ERR(core_ck)) {
		printk(KERN_ERR "failed to get core_ck\n");
	}
	
	clkout2_src_ck = clk_get(NULL, "clkout2_src_ck");
	if (IS_ERR(clkout2_src_ck)) {
		printk(KERN_ERR "failed to get clkout2_src_ck\n");
	}
	
	sys_clkout2 = clk_get(NULL, "sys_clkout2");
	if (IS_ERR(sys_clkout2)) {
		printk(KERN_ERR "failed to get sys_clkout2\n");
	}
	
	if ( clk_set_parent(clkout2_src_ck, core_ck) != 0) {
		printk(KERN_ERR "failed to set sys_clkout2 parent to clkout2\n");
	}
	
	/* Set the clock to 12 Mhz */
	clk_set_rate(sys_clkout2, 12000000);

	clk_put(sys_clkout2);
	clk_put(clkout2_src_ck);
	clk_put(core_ck);

	/* McBSP2 mux */
	omap_mux_init_signal("mcbsp2_clkx", OMAP_PIN_INPUT);
	omap_mux_init_signal("mcbsp2_dr", OMAP_PIN_INPUT);
	omap_mux_init_signal("mcbsp2_dx", OMAP_PIN_OUTPUT);
	omap_mux_init_signal("mcbsp2_fsx", OMAP_PIN_INPUT);

	/* McBSP3 mux */
	/* omap_mux_init_signal("mcbsp3_clkx", OMAP_PIN_INPUT);
	omap_mux_init_signal("mcbsp3_dr", OMAP_PIN_INPUT);
	omap_mux_init_signal("mcbsp3_dx", OMAP_PIN_OUTPUT);
	omap_mux_init_signal("mcbsp3_fsx", OMAP_PIN_INPUT);
	*/

	/* McBSP4 mux */
	omap_mux_init_signal("mcbsp4_clkx.mcbsp4_clkx", OMAP_PIN_INPUT);
	omap_mux_init_signal("mcbsp4_dr.mcbsp4_dr", OMAP_PIN_INPUT);
	omap_mux_init_signal("mcbsp4_dx.mcbsp4_dx", OMAP_PIN_OUTPUT);
	omap_mux_init_signal("mcbsp4_fsx.mcbsp4_fsx", OMAP_PIN_INPUT);

	if (audio_gpio.spdif != -1)
		archos_gpio_init_output( audio_gpio.spdif, "spdif" );

	if (audio_gpio.hp_on != -1)
		archos_gpio_init_output( audio_gpio.hp_on, "hp_on" );

	if (audio_gpio.headphone_plugged != -1)
		archos_gpio_init_input( audio_gpio.headphone_plugged, "hp_detect" );

	if (audio_gpio.vamp_vbat != -1)
		archos_gpio_init_output( audio_gpio.vamp_vbat, "vamp_vbat" );
	if (audio_gpio.vamp_dc != -1)
		archos_gpio_init_output( audio_gpio.vamp_dc, "vamp_dc" );
	
	// XXX maybe prevents OFF mode?
	if (audio_gpio.headphone_plugged != -1)
		gpio_set_debounce(audio_gpio.headphone_plugged, 1);

	return platform_device_register(&audio_soc_device);
}

EXPORT_SYMBOL(archos_audio_wm8988_get_io);
