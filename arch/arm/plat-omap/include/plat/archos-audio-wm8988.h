#ifndef _ARCH_ARCHOS_AUDIO_WM8988_H_
#define _ARCH_ARCHOS_AUDIO_WM8988_H_

struct audio_wm8988_device_config {
	void (*set_spdif)(int onoff);
	int (*get_headphone_plugged)(void);
	int (*get_headphone_irq)(void);
	void (*set_codec_master_clk_state)(int state);
	int (*get_master_clock_rate)(void);
	void (*set_speaker_state)(int state);
	void (*suspend)(void);
	void (*resume)(void);
};

struct audio_wm8988_device_config * archos_audio_wm8988_get_io(void);

int __init archos_audio_wm8988_init(void);

#endif
