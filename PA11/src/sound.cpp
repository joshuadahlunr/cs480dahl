#include "sound.h"
#include <iostream>

#define MINIAUDIO_IMPLEMENTATION
#include "../thirdparty/miniaudio.h"

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

SoundEffect::SoundEffect(const char* filePath) {
	//Decoder reads in file
	userData.decoder = new ma_decoder();
	ma_result result = ma_decoder_init_file(filePath, NULL, userData.decoder);
	if(result != MA_SUCCESS) {
		std::cout << "ERROR INITIALIZING SOUND: " << filePath << std::endl;
		return;
	}
	//There are the default settings
	config = ma_device_config_init(ma_device_type_playback);
	config.playback.format	  = userData.decoder->outputFormat;
	config.playback.channels = userData.decoder->outputChannels;
	config.sampleRate		  = userData.decoder->outputSampleRate;
	config.dataCallback	  = data_callback;
	config.pUserData		  = &userData;
	//Device controls starting and stopping
	device = new ma_device();
	if (ma_device_init(NULL, &config, device) != MA_SUCCESS) {
		std::cout << "Failed to open playback device for " << filePath << std::endl;
		ma_decoder_uninit(userData.decoder);
	}
}

//Declare new sounds here and add them to the map. That's all you have to do.
Sound::Sound() {
	// soundEffects["Music"] = new SoundEffect("../sound/switch-me-on.mp3");
	// soundEffects["Charging"] = new SoundEffect("../sound/mixkit-arcade-rising-231.wav");
	// soundEffects["Bounce"] = new SoundEffect("../sound/mixkit-arcade-bonus-alert-767.wav");
	// soundEffects["LoseBall"] = new SoundEffect("../sound/mixkit-retro-arcade-lose-2027.wav");
	soundEffects["Music"] = new SoundEffect("../sounds/410574__yummie__game-background-music-loop-short.mp3");
	soundEffects["Abducting"] = new SoundEffect("../sounds/505379__bloodpixelhero__alien-alarm.wav");
	soundEffects["Score"] = new SoundEffect("../sounds/mixkit-retro-game-notification-212.wav");
	soundEffects["Penalty"] = new SoundEffect("../sounds/mixkit-failure-arcade-alert-notification-240.wav");
}

Sound::~Sound() {
	//Decoders and devices must be uninitialized
	for(auto iter = soundEffects.begin(); iter != soundEffects.end(); iter++) {
		SoundEffect *effect = iter->second;
		stopSound(iter->first);
		ma_device_uninit(effect->getDevice());
		ma_decoder_uninit(effect->getDecoder());
	}
}

void Sound::startSound(std::string key, bool fromBeginning, bool looping) {
	SoundEffect* effect = soundEffects[key];
	// If the effect is already playing and we aren't starting it over... abort!
	if(effect->isPlaying && !fromBeginning)
		return;

	//Starting a sound that's already started crashes the program
	if(ma_device_is_started(effect->getDevice())) {
		ma_device_stop(effect->getDevice());
	}
	//Reset playback position if desired
	if(fromBeginning) ma_decoder_seek_to_pcm_frame(effect->getDecoder(), 0);
	// Mark if the sound should loop or not
	effect->setLooping(looping);
	//Uninitialize if it fails to start, something must be wrong
	if(ma_device_start(effect->getDevice()) != MA_SUCCESS) {
		ma_device_uninit(effect->getDevice());
		ma_decoder_uninit(effect->getDecoder());
	// If it starts then mark the effect as playing
	} else
		effect->isPlaying = true;
}

void Sound::stopSound(std::string key) {
	SoundEffect* effect = soundEffects[key];
	ma_device_stop(effect->getDevice());
	// Mark the effect as no longer playing
	effect->isPlaying = false;
}

void Sound::stopAllSounds() {
	for(auto iter = soundEffects.begin(); iter != soundEffects.end(); iter++) {
		stopSound(iter->first);
	}
}

// Callback function which provides more audio data to a sound device when it runs out
void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    SoundEffect::UserData* data = (SoundEffect::UserData*)pDevice->pUserData;
    if (data == NULL)
        return;

    // A decoder is a data source which means you can seamlessly plug it into the ma_data_source API. We can therefore take advantage of the "loop" parameter of ma_data_source_read_pcframes() to handle looping for us.
    ma_data_source_read_pcm_frames(data->decoder, pOutput, frameCount, NULL, (data->looping ? MA_TRUE : MA_FALSE));

    (void)pInput;
}
