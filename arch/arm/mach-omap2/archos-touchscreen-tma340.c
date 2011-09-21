/*
 *    archos-touchscreen-tm340.c : 17/03/2011
 *    g.revaillot, revaillot@archos.com
 */

#include <linux/err.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/machine.h>
#include <linux/stat.h>
#include <linux/types.h>

#include <mach/board-archos.h>
#include <mach/gpio.h>
#include <plat/archos-gpio.h>

#include <linux/input/cypress-tma340.h>

#include "mux.h"

static struct regulator_consumer_supply tsp_vcc_consumer[] = {
	REGULATOR_SUPPLY("tsp_vcc", "4-0024"),
};
static struct regulator_init_data fixed_reg_tsp_vcc_initdata = {
	.constraints = {
		.min_uV = 3300000,
		.max_uV = 3300000,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
	},
	
	.supply_regulator = "VCC",
	.consumer_supplies = tsp_vcc_consumer,
	.num_consumer_supplies = ARRAY_SIZE(tsp_vcc_consumer),
};
static struct fixed_voltage_config fixed_reg_tsp_vcc = {
	.supply_name	= "TSP_VCC",
	.microvolts	= 3300000,
	.gpio		= -EINVAL,
	.enable_high	= 1,
	.enabled_at_boot= 0,
	.init_data	= &fixed_reg_tsp_vcc_initdata,
};
static struct platform_device fixed_supply_tsp_vcc = {
	.name 	= "reg-fixed-voltage",
	.id = 6,
	.dev.platform_data = &fixed_reg_tsp_vcc,
};

int __init archos_touchscreen_tm340_init(struct cypress_tma340_platform_data *pdata)
{
	const struct archos_i2c_tsp_config *tsp_config;
	const struct archos_i2c_tsp_conf *conf;
	int ret;

	tsp_config = omap_get_config(ARCHOS_TAG_I2C_TSP,
			struct archos_i2c_tsp_config);
	if (tsp_config == NULL)
		return -ENODEV;

	conf = hwrev_ptr(tsp_config, hardware_rev);

	if (IS_ERR(conf)) {
		pr_err("%s: no device configuration for hardware_rev %i\n",
				__FUNCTION__, hardware_rev);	
		return -ENODEV;
	}

	if (conf->pwr_gpio > 0) {
		if (conf->pwr_signal) {
			omap_mux_init_signal(conf->pwr_signal, PIN_OUTPUT);
		} else {
			omap_mux_init_gpio(conf->pwr_gpio, PIN_OUTPUT);
		}

		fixed_reg_tsp_vcc.gpio = conf->pwr_gpio;
		platform_device_register(&fixed_supply_tsp_vcc);
	} else {
		pr_err("%s: ts pwron gpio is not valid.\n", __FUNCTION__);	
		return -ENODEV;
	}

	if (conf->irq_gpio > 0) {
		ret = gpio_request(conf->irq_gpio, "tma340_ts_irq");

		if (!ret) {
			gpio_direction_input(conf->irq_gpio);

			if (conf->irq_signal)
				omap_mux_init_signal(conf->irq_signal,
						OMAP_PIN_INPUT_PULLUP);
			else
				omap_mux_init_gpio(conf->irq_gpio, PIN_INPUT);

			gpio_export(conf->irq_gpio, false);
		} else {
			printk("%s : could not request irq gpio %d\n",
					__FUNCTION__, conf->irq_gpio);
			return ret;
		}

		if (conf->irq_gpio!= -1) {
			pdata->irq = gpio_to_irq(conf->irq_gpio);
		} else {
			pdata->irq = -1;
		}
	} else {
		pr_err("%s: ts irq gpio is not valid.\n", __FUNCTION__);	
		return -ENODEV;
	}

	printk(KERN_DEBUG "%s: irq_gpio %d - irq %d, pwr_gpio %d\n",
			__FUNCTION__, pdata->irq, conf->irq_gpio, conf->pwr_gpio);

	return 0;
}
