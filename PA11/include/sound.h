#ifndef SOUND_H
#define SOUND_H

#include "../thirdparty/miniaudio.h"
#include <map>
#include <string>

//One individual sound effect
class SoundEffect {
friend class Sound;
public:
	// Struct storing the data which is provided to the data callback
	struct UserData {
		ma_decoder* decoder;
		bool looping = false;
	};
public:
	SoundEffect(const char* filePath);
	ma_device* getDevice() { return device; }
	ma_decoder* getDecoder() { return userData.decoder; }

	void setLooping(bool looping) { userData.looping = looping; }

private:
	UserData userData;
	ma_device_config config;
	ma_device* device;
	const char* filePath;

	bool isPlaying = false;
};

//Stores sound effects. Starts and stops sounds
class Sound {
public:
	Sound();
	~Sound();

	void startSound(std::string key, bool fromBeginning = true, bool looping = false);
	void stopSound(std::string key);
	void stopAllSounds();

private:
	std::map<std::string, SoundEffect*> soundEffects;
};

#endif
