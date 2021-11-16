#ifndef SOUND_H
#define SOUND_H

#include "../thirdparty/miniaudio.h"

class Sound {
	public:
		Sound();
		~Sound();

		void setCharging(bool val);
		void bounce();
		void loseBall();

		bool gameRunning = true;

	private:
		ma_device musicDevice;
		ma_decoder musicDecoder;
		ma_device_config musicConfig;
		const char* musicPath = "../sound/switch-me-on.mp3";

		ma_device chargingDevice;
		ma_decoder chargingDecoder;
		ma_device_config chargingConfig;
		const char* chargingPath = "../sound/mixkit-arcade-rising-231.wav";

		ma_device bounceDevice;
		ma_decoder bounceDecoder;
		ma_device_config bounceConfig;
		const char* bouncePath = "../sound/mixkit-arcade-bonus-alert-767.wav";

		ma_device loseBallDevice;
		ma_decoder loseBallDecoder;
		ma_device_config loseBallConfig;
		const char* loseBallPath = "../sound/mixkit-retro-arcade-lose-2027.wav";
};

#endif
