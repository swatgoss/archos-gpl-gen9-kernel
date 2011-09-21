#if defined(CONFIG_USB_STORAGE_JM20329) || \
		defined(CONFIG_USB_STORAGE_JM20329_MODULE)

UNUSUAL_DEV(  0x152d, 0x2329, 0x0100, 0x0200,
		"JMicron",
		"USB to ATA/ATAPI Bridge",
		US_SC_DEVICE, US_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE | US_FL_SANE_SENSE ),

#endif