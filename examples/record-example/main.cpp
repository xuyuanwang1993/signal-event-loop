/*
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/
#include <SDL2/SDL.h>

#include <stdlib.h>
#include<unistd.h>
#include<string>
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif
typedef unsigned char ID[4];

typedef struct
{
    ID          chunkID;  /* {'f', 'm', 't', ' '} */
    uint32_t    chunkSize;

    uint16_t    audioFormat;
    uint16_t    numChannels;
    uint32_t    sampleRate;
    uint32_t    byteRate;
    uint16_t    blockAlign;
    uint16_t    bitsPerSample;
} FormatChunk;

typedef struct
{
    ID             chunkID;  /* {'d', 'a', 't', 'a'}  */
    uint32_t       chunkSize;
    unsigned char  data[];
} DataChunk;
static FormatChunk formatchunk;
static DataChunk   datachunk;
static FILE *wav_fp=nullptr;
static uint32_t  pcm_filed_size;
static SDL_AudioSpec spec;
static SDL_AudioDeviceID devid = 0;
const char * file_name="output.wav";
static void init()
{
    if(!wav_fp)
    {
        wav_fp=fopen(file_name,"wb");
        if(!wav_fp)return;
        printf("open %s \r\n",file_name);
    }
    memset(&formatchunk,0,sizeof (formatchunk));
    memset(&datachunk,0,sizeof (datachunk));
    fwrite("RIFF", 1, 4, wav_fp);
    fwrite("xxxx", 1, 4, wav_fp);  //reserved for the total chunk size
    fwrite("WAVE", 1, 4, wav_fp);

    formatchunk.chunkID[0] = 'f';
    formatchunk.chunkID[1] = 'm';
    formatchunk.chunkID[2] = 't';
    formatchunk.chunkID[3] = ' ';
    formatchunk.chunkSize  = 16;
    formatchunk.audioFormat = 1;
    formatchunk.numChannels=spec.channels;
    formatchunk.sampleRate=spec.freq;
    formatchunk.bitsPerSample=spec.format&SDL_AUDIO_MASK_BITSIZE;
    formatchunk.byteRate = spec.freq * spec.channels * (spec.format&SDL_AUDIO_MASK_BITSIZE)/8;
    formatchunk.blockAlign = spec.channels * (spec.format&SDL_AUDIO_MASK_BITSIZE)/8;
    fwrite(&formatchunk, 1, sizeof(formatchunk), wav_fp);
    datachunk.chunkID[0] = 'd';
    datachunk.chunkID[1] = 'a';
    datachunk.chunkID[2] = 't';
    datachunk.chunkID[3] = 'a';
    datachunk.chunkSize = 0;
    fwrite(&datachunk, 1, sizeof(ID)+sizeof(uint32_t), wav_fp);
}

static void deinit()
{
    if(!wav_fp)return;
    //write
    fseek(wav_fp, 12+sizeof(formatchunk), SEEK_SET);
    datachunk.chunkSize=pcm_filed_size;
    fwrite(&datachunk, 1, sizeof(ID)+sizeof(uint32_t), wav_fp);

    fseek(wav_fp, 4, SEEK_SET);
    uint32_t chunk_size = 4/*WAVE*/ +sizeof (formatchunk)+sizeof (datachunk)+pcm_filed_size;;
    fwrite(&chunk_size, 1, 4, wav_fp);
    fclose(wav_fp);
    wav_fp=nullptr;
}

void audioRecordingCallback( void* userdata, Uint8* stream, int len )
{

    //copy audio from stream
    if(!wav_fp)
    {
        init();
    }
    if(wav_fp)
    {
        pcm_filed_size+=len;
        fwrite(stream,1,len,wav_fp);
    }
}



static void
exit_sdl()
{

    /* stop playing back, quit. */
    deinit();
    SDL_Log("Shutting down.\n");
    SDL_PauseAudioDevice(devid, 1);
    SDL_CloseAudioDevice(devid);
    SDL_Quit();
#ifdef __EMSCRIPTEN__
    emscripten_cancel_main_loop();
#endif
}

int
main(int argc, char **argv)
{
    /* (argv[1] == NULL means "open default device.") */
    const char *devname = argv[1];
    int record_time=15;
    if(argc>=2)
    {
        record_time=std::stoi(argv[2]);
        if(record_time<5)record_time=5;
        if(record_time>6000)record_time=6000;
    }
    SDL_AudioSpec wanted;
    int devcount;
    int i;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    /* Load the SDL library */
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s\n", SDL_GetError());
        return (1);
    }
    SDL_Log("Using audio driver: %s\n", SDL_GetCurrentAudioDriver());

    devcount = SDL_GetNumAudioDevices(SDL_TRUE);
    for (i = 0; i < devcount; i++) {
        SDL_Log(" Capture device #%d: '%s'\n", i, SDL_GetAudioDeviceName(i, SDL_TRUE));
    }

    SDL_zero(wanted);
    wanted.freq = 48000;
    wanted.format = AUDIO_S16;
    wanted.channels = 2;
    wanted.samples = 4096;
    wanted.callback = audioRecordingCallback;
    static char user_data[5]="test";
    wanted.userdata=user_data;
    SDL_zero(spec);

    /* DirectSound can fail in some instances if you open the same hardware
       for both capture and output and didn't open the output end first,
       according to the docs, so if you're doing something like this, always
       open your capture devices second in case you land in those bizarre
       circumstances. */

    SDL_Log("Opening capture device %s%s%s...\n",
            devname ? "'" : "",
            devname ? devname : "[[default]]",
            devname ? "'" : "");

    devid = SDL_OpenAudioDevice(argv[1], SDL_TRUE, &wanted, &spec, 0);
    if (!devid) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open an audio device for capture: %s!\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }
    printf("%d  %d %d %d %d\r\n",devid,spec.channels,spec.freq,spec.samples,spec.format&0xff);

    SDL_PauseAudioDevice(devid, SDL_FALSE);
    sleep(record_time);
    exit_sdl();
    return 0;
}

