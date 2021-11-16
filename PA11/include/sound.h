#ifndef SOUND_H
#define SOUND_H

#include "../thirdparty/miniaudio.h"
#include <map>
#include <string>

//One individual sound effect
class SoundEffect {
	public:
		SoundEffect(const char* filePath);
		ma_device* GetDevice() { return m_device; }
		ma_decoder* GetDecoder() { return m_decoder; }
	private:
		ma_decoder* m_decoder;
		ma_device_config m_config;
		ma_device* m_device;
		const char* m_filePath;
};

//Stores sound effects. Starts and stops sounds
class Sound {
	public:
		Sound();
		~Sound();

		void StartSound(std::string key, bool fromBeginning = true);
		void StopSound(std::string key);
		void StopAllSounds();

	private:
		std::map<std::string, SoundEffect*> soundEffects;
};

#endif
