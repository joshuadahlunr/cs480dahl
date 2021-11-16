#include "sound.h"
#include <iostream>

#define MINIAUDIO_IMPLEMENTATION
#include "../thirdparty/miniaudio.h"

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

SoundEffect::SoundEffect(const char* filePath) {
	//Decoder reads in file
	m_decoder = new ma_decoder();
	ma_result result = ma_decoder_init_file(filePath, NULL, m_decoder);
	if(result != MA_SUCCESS) {
		std::cout << "ERROR INITIALIZING SOUND: " << filePath << std::endl;
		return;
	}
	//There are the default settings
	m_config = ma_device_config_init(ma_device_type_playback);
	m_config.playback.format	  = m_decoder->outputFormat;
	m_config.playback.channels = m_decoder->outputChannels;
	m_config.sampleRate		  = m_decoder->outputSampleRate;
	m_config.dataCallback	  = data_callback;
	m_config.pUserData		  = m_decoder;
	//Device controls starting and stopping
	m_device = new ma_device();
	if (ma_device_init(NULL, &m_config, m_device) != MA_SUCCESS) {
		std::cout << "Failed to open playback device for " << filePath << std::endl;
		ma_decoder_uninit(m_decoder);
	}
}

//Declare new sounds here and add them to the map. That's all you have to do.
Sound::Sound() {
	soundEffects["Music"] = new SoundEffect("../sound/switch-me-on.mp3");
	soundEffects["Charging"] = new SoundEffect("../sound/mixkit-arcade-rising-231.wav");
	soundEffects["Bounce"] = new SoundEffect("../sound/mixkit-arcade-bonus-alert-767.wav");
	soundEffects["LoseBall"] = new SoundEffect("../sound/mixkit-retro-arcade-lose-2027.wav");
}

Sound::~Sound() {
	//Decoders and devices must be uninitialized
	for(auto iter = soundEffects.begin(); iter != soundEffects.end(); iter++) {
		SoundEffect *effect = iter->second;
		StopSound(iter->first);
		ma_device_uninit(effect->GetDevice());
		ma_decoder_uninit(effect->GetDecoder());
	}
}

void Sound::StartSound(std::string key, bool fromBeginning) {

	SoundEffect* effect = soundEffects[key];
	//Starting a sound that's already started crashes the program
	if(ma_device_is_started(effect->GetDevice())) {
		ma_device_stop(effect->GetDevice());
	}
	//Reset playback position if desired
	if(fromBeginning) ma_decoder_seek_to_pcm_frame(effect->GetDecoder(), 0);
	//Uninitialize if it fails to start, something must be wrong
	if(ma_device_start(effect->GetDevice()) != MA_SUCCESS) {
		ma_device_uninit(effect->GetDevice());
		ma_decoder_uninit(effect->GetDecoder());
	}
}

void Sound::StopSound(std::string key) {
	ma_device_stop(soundEffects[key]->GetDevice());
}

void Sound::StopAllSounds() {
	for(auto iter = soundEffects.begin(); iter != soundEffects.end(); iter++) {
		StopSound(iter->first);
	}
}

//This was provided in the example code. Not sure what it does.
void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
	ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
	if (pDecoder == NULL) {
		return;
	}

	ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount);

	(void)pInput;
}
