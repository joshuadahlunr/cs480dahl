#include "sound.h"
#include <iostream>

#define MINIAUDIO_IMPLEMENTATION
#include "../thirdparty/miniaudio.h"

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

Sound::Sound() { 

    // ma_decoder decoder;
    ma_result result = ma_decoder_init_file(musicPath, NULL, &musicDecoder);
    if (result != MA_SUCCESS) {
        std::cout << "ERROR INITIALIZING SOUND" << std::endl;
    }

    musicConfig = ma_device_config_init(ma_device_type_playback);
    musicConfig.playback.format   = musicDecoder.outputFormat;
    musicConfig.playback.channels = musicDecoder.outputChannels;
    musicConfig.sampleRate        = musicDecoder.outputSampleRate;
    musicConfig.dataCallback      = data_callback;
    musicConfig.pUserData         = &musicDecoder;

    // ma_device device;
    if (ma_device_init(NULL, &musicConfig, &musicDevice) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        ma_decoder_uninit(&musicDecoder);
        std::cout << "ERROR INITIALIZING SOUND" << std::endl;
    }

    if (ma_device_start(&musicDevice) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&musicDevice);
        ma_decoder_uninit(&musicDecoder);
        std::cout << "ERROR INITIALIZING SOUND" << std::endl;
    }

    // ma_decoder decoder;
    result = ma_decoder_init_file(chargingPath, NULL, &chargingDecoder);
    if (result != MA_SUCCESS) {
        std::cout << "ERROR INITIALIZING SOUND" << std::endl;
    }

    chargingConfig = ma_device_config_init(ma_device_type_playback);
    chargingConfig.playback.format   = chargingDecoder.outputFormat;
    chargingConfig.playback.channels = chargingDecoder.outputChannels;
    chargingConfig.sampleRate        = chargingDecoder.outputSampleRate;
    chargingConfig.dataCallback      = data_callback;
    chargingConfig.pUserData         = &chargingDecoder;

    // ma_device device;
    if (ma_device_init(NULL, &chargingConfig, &chargingDevice) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        ma_decoder_uninit(&chargingDecoder);
        std::cout << "ERROR INITIALIZING SOUND" << std::endl;
    }

    result = ma_decoder_init_file(bouncePath, NULL, &bounceDecoder);
    if (result != MA_SUCCESS) {
        std::cout << "ERROR INITIALIZING SOUND" << std::endl;
    }

    bounceConfig = ma_device_config_init(ma_device_type_playback);
    bounceConfig.playback.format   = bounceDecoder.outputFormat;
    bounceConfig.playback.channels = bounceDecoder.outputChannels;
    bounceConfig.sampleRate        = bounceDecoder.outputSampleRate;
    bounceConfig.dataCallback      = data_callback;
    bounceConfig.pUserData         = &bounceDecoder;

    // ma_device device;
    if (ma_device_init(NULL, &bounceConfig, &bounceDevice) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        ma_decoder_uninit(&bounceDecoder);
        std::cout << "ERROR INITIALIZING SOUND" << std::endl;
    }


    result = ma_decoder_init_file(loseBallPath, NULL, &loseBallDecoder);
    if (result != MA_SUCCESS) {
        std::cout << "ERROR INITIALIZING SOUND" << std::endl;
    }

    loseBallConfig = ma_device_config_init(ma_device_type_playback);
    loseBallConfig.playback.format   = loseBallDecoder.outputFormat;
    loseBallConfig.playback.channels = loseBallDecoder.outputChannels;
    loseBallConfig.sampleRate        = loseBallDecoder.outputSampleRate;
    loseBallConfig.dataCallback      = data_callback;
    loseBallConfig.pUserData         = &loseBallDecoder;

    // ma_device device;
    if (ma_device_init(NULL, &loseBallConfig, &loseBallDevice) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        ma_decoder_uninit(&loseBallDecoder);
        std::cout << "ERROR INITIALIZING SOUND" << std::endl;
    }
}

Sound::~Sound() {
    ma_device_uninit(&musicDevice);
    ma_decoder_uninit(&musicDecoder);

    ma_device_uninit(&chargingDevice);
    ma_decoder_uninit(&chargingDecoder);
}

void Sound::SetCharging(bool val) {
    if(val == ma_device_is_started(&chargingDevice) || !gameRunning) return;

    if(val) {

        ma_decoder_seek_to_pcm_frame(&chargingDecoder, 0);

        if (ma_device_start(&chargingDevice) != MA_SUCCESS) {
            printf("Failed to start playback device.\n");
            ma_device_uninit(&chargingDevice);
            ma_decoder_uninit(&chargingDecoder);
            std::cout << "ERROR INITIALIZING SOUND" << std::endl;
        }
    } else {
        ma_device_stop(&chargingDevice);
    }
}

void Sound::Bounce() {
    if(ma_device_is_started(&bounceDevice)) {
        ma_device_stop(&bounceDevice);
    }

    ma_decoder_seek_to_pcm_frame(&bounceDecoder, 0);

    if (ma_device_start(&bounceDevice) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&bounceDevice);
        ma_decoder_uninit(&bounceDecoder);
        std::cout << "ERROR INITIALIZING SOUND" << std::endl;
    }
}

void Sound::LoseBall() {
    if(ma_device_is_started(&loseBallDevice)) {
        ma_device_stop(&loseBallDevice);
    }

    ma_decoder_seek_to_pcm_frame(&loseBallDecoder, 0);

    if (ma_device_start(&loseBallDevice) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&loseBallDevice);
        ma_decoder_uninit(&loseBallDecoder);
        std::cout << "ERROR INITIALIZING SOUND" << std::endl;
    }
}

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }

    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount);

    (void)pInput;
}