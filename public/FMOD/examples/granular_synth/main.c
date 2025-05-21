/*===============================================================================================
 Granular Synthesis example.
 Copyright (c), Firelight Technologies Pty, Ltd 2004-2016.

 This example shows how you can play a string of sounds together without gaps, using the setDelay
 command.
 The basic operation is:
 1. play 2 sounds initially at the same time, the first sound immediately, and the 2nd sound
    with a delay calculated by the length of the first sound.
 2. call setDelay to initiate the delayed playback.  setDelay is sample accurate.  setDelay uses
    -output- samples as the time frame, not source samples.  These samples are a fixed amount
    per second regardless of the source sound format, for example, 48000 samples per second if 
    FMOD is initialized to 48khz output.
 3. Output samples are calculated from source samples with a simple source->output sample rate 
    conversion. ie
        sound_length *= outputrate;   
        sound_length /= (int)sound_frequency;
 4. When the first sound finishes, the second one should have automatically started.  This is
    a good oppurtunity to queue up the next sound.  Repeat step 2.
 5. Make sure the framerate is high enough to queue up a new sound before the other one finishes
    otherwise you will get gaps.
 
 These sounds are not limited by format, channel count or bit depth like the realtimestitching
 example is, and can also be modified to allow for overlap, by reducing the delay from the first
 sound playing to the second by the overlap amount.
 
 #define USE_STREAMS = use 2 stream instances, create and free them while they play.
 //#define USE_STREAMS = use 6 static wavs, all loaded into memory.
 

===============================================================================================*/
#include <windows.h>
#include <stdio.h>
#include <conio.h>

#include "../../api/inc/fmod.h"
#include "../../api/inc/fmod_errors.h"

void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }
}

//#define USE_STREAMS

FMOD_SYSTEM *gSystem;

#ifdef USE_STREAMS
    #define NUMSOUNDS 3                         /* Use some longer sounds, free and load them on the fly. */
    FMOD_SOUND       *sound[2] = { 0,0 };       /* 2 streams active, double buffer them. */
    const char       *soundname[NUMSOUNDS] = 
    {
        "../media/c.ogg",
        "../media/d.ogg",
        "../media/e.ogg",
    };
#else
    #define NUMSOUNDS 6                                     /* These sounds will be loaded into memory statically. */
    FMOD_SOUND       *sound[NUMSOUNDS] = { 0,0,0,0,0,0 };   /* 8 sounds active, one for each wav. */
    const char       *soundname[NUMSOUNDS] = 
    {
        "../media/granular/truck_idle_off_01.wav",
        "../media/granular/truck_idle_off_02.wav",
        "../media/granular/truck_idle_off_03.wav",
        "../media/granular/truck_idle_off_04.wav",
        "../media/granular/truck_idle_off_05.wav",
        "../media/granular/truck_idle_off_06.wav"
    };
#endif

void set_paused(FMOD_CHANNEL **channel, int paused)
{
    FMOD_RESULT result;
    unsigned int blocksize;
    
    result = FMOD_System_GetDSPBufferSize(gSystem, &blocksize, 0);
    ERRCHECK(result);

    if (!paused)
    {
        unsigned int pausestart_hi = 0, pausestart_lo = 0;

        FMOD_System_GetDSPClock(gSystem, &pausestart_hi, &pausestart_lo);
        
        FMOD_64BIT_ADD(pausestart_hi, pausestart_lo, 0, blocksize * 2);   /* Into the future by 2 mixer blocks. */
        printf("\npause BOTH at %d \n", pausestart_lo);
       
        /* Make them both pause at exactly the same tick.  Mute them both to avoid a click as well. */
        FMOD_Channel_SetMute(channel[0], TRUE);
        FMOD_Channel_SetDelay(channel[0], FMOD_DELAYTYPE_DSPCLOCK_PAUSE, pausestart_hi, pausestart_lo);
        FMOD_Channel_SetMute(channel[1], TRUE);
        FMOD_Channel_SetDelay(channel[1], FMOD_DELAYTYPE_DSPCLOCK_PAUSE, pausestart_hi, pausestart_lo);
    }
    else
    {
        unsigned int syshi, syslo;
        int count;

        FMOD_System_GetDSPClock(gSystem, &syshi, &syslo);

        printf("\nunpause BOTH at %d\n", syslo);
        
        for (count = 0; count < 2; count++)
        { 
            unsigned int starttime_hi, starttime_lo; 
            unsigned int pausetime_hi = 0, pausetime_lo = 0;
            unsigned int hi = syshi, lo = syslo;
            
            FMOD_Channel_GetDelay(channel[count], FMOD_DELAYTYPE_DSPCLOCK_PAUSE, &pausetime_hi, &pausetime_lo);
            FMOD_Channel_GetDelay(channel[count], FMOD_DELAYTYPE_DSPCLOCK_START, &starttime_hi, &starttime_lo);

            FMOD_64BIT_ADD(hi, lo, 0, blocksize * 2);                   /* Push operation into the future by 2 mixer blocks so it doesnt conflict with mixer. */
            if (starttime_lo > pausetime_lo)                            /* Was already playing, unpause immediately. */
            {
                FMOD_64BIT_ADD(hi, lo, starttime_hi, starttime_lo);     /* Push forward the delayed start by the gap between starting and pausing */
                FMOD_64BIT_SUB(hi, lo, pausetime_hi, pausetime_lo);     /* Push forward the delayed start by the gap between starting and pausing */
            }
            printf("restart %d at %d\n", count, lo);
            FMOD_Channel_SetDelay(channel[count], FMOD_DELAYTYPE_DSPCLOCK_PAUSE, 0, 0);
            FMOD_Channel_SetDelay(channel[count], FMOD_DELAYTYPE_DSPCLOCK_START, hi, lo);
            FMOD_Channel_SetMute(channel[count], FALSE);
            FMOD_Channel_SetPaused(channel[count], FALSE);
        }
    }
}


FMOD_CHANNEL *queue_next_sound(int outputrate, FMOD_CHANNEL *playingchannel, int newindex, int slot)
{
    FMOD_RESULT result;
    FMOD_CHANNEL *newchannel;
    FMOD_SOUND *newsound;
    
    #ifdef USE_STREAMS                                  /* Create a new stream */
    FMOD_CREATESOUNDEXINFO info;
    memset(&info, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    info.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    info.suggestedsoundtype = FMOD_SOUND_TYPE_OGGVORBIS;
    result = FMOD_System_CreateStream(gSystem, soundname[newindex], FMOD_IGNORETAGS | FMOD_LOWMEM, &info, &sound[slot]);
    ERRCHECK(result);
    newsound = sound[slot];
    #else                                               /* Use an existing sound that was passed into us */
    newsound = sound[newindex];
    #endif
    
    result = FMOD_System_PlaySound(gSystem, FMOD_CHANNEL_FREE, newsound, 1, &newchannel);
    ERRCHECK(result);
      
    result = FMOD_Channel_SetSpeakerMix(newchannel, 1,1,1,1,1,1,1,1);
    ERRCHECK(result);
           
    
    if (playingchannel)
    {    
        unsigned int hi = 0, lo = 0, sound_length;
        float sound_frequency;
        FMOD_SOUND *playingsound;
        
        /*
            Get the start time of the playing channel.
        */
        result = FMOD_Channel_GetDelay(playingchannel, FMOD_DELAYTYPE_DSPCLOCK_START, &hi, &lo);
        ERRCHECK(result);
        
        printf("playing sound started at %d\n", lo);
        
        /*
            Grab the length of the playing sound, and its frequency, so we can caluate where to place the new sound on the time line.
        */
        result = FMOD_Channel_GetCurrentSound(playingchannel, &playingsound);
        ERRCHECK(result);
        result = FMOD_Sound_GetLength(playingsound, &sound_length, FMOD_TIMEUNIT_PCM);
        ERRCHECK(result);
        result = FMOD_Channel_GetFrequency(playingchannel, &sound_frequency);
        ERRCHECK(result);
        
        /* 
            Now calculate the length of the sound in 'output samples'.  
            Ie if a 44khz sound is 22050 samples long, and the output rate is 48khz, then we want to delay by 24000 output samples.
        */
        sound_length *= outputrate;   
        sound_length /= (int)sound_frequency;
        
        FMOD_64BIT_ADD(hi, lo, 0, sound_length);  /* Add output rate adjusted sound length, to the clock value of the sound that is currently playing */
            
        result = FMOD_Channel_SetDelay(newchannel, FMOD_DELAYTYPE_DSPCLOCK_START, hi, lo);      /* Set the delay of the new sound to the end of the old sound */
        ERRCHECK(result);
    }
    
    {
        unsigned int hi = 0, lo = 0;
        float val, variation;
        
        /*
            Randomize pitch/volume to make it sound more realistic / random.
        */
        FMOD_Channel_GetFrequency(newchannel, &val);
        variation = (((float)(rand()%10000) / 5000.0f) - 1.0f); /* -1.0 to +1.0 */
        val *= (1.0f + (variation * 0.02f));                    /* @22khz, range fluctuates from 21509 to 22491 */
        FMOD_Channel_SetFrequency(newchannel, val);

        FMOD_Channel_GetVolume(newchannel, &val);
        variation = ((float)(rand()%10000) / 10000.0f);         /*  0.0 to 1.0 */
        val *= (1.0f - (variation * 0.2f));                     /*  0.8 to 1.0 */
        FMOD_Channel_SetVolume(newchannel, val);
                
        FMOD_Channel_GetDelay(newchannel, FMOD_DELAYTYPE_DSPCLOCK_START, &hi, &lo);
        printf("new sound to start at %d (slot %d)\n", lo, slot);
    }   
        
    result = FMOD_Channel_SetPaused(newchannel, FALSE);
    ERRCHECK(result);
       
    return newchannel;
}

int main(int argc, char *argv[])
{
    FMOD_CHANNEL    *channel[2] = { 0,0 };
    FMOD_RESULT       result;
    int               key, outputrate, slot = 0, count, numdrivers;
    unsigned int      version;
    
    printf("=============================================================================\n");
    printf("Granular Synthesis SetDelay example.\n");
    printf("Copyright (c) Firelight Technologies 2004-2016.\n");
    printf("=============================================================================\n");
    printf("\n");
    printf("TOGGLE #define USE_STREAMS ON/OFF TO SWITCH BETWEEN STREAMS/STATIC SAMPLES.\n");
    printf("Press space to pause, Esc to quit\n");
    printf("\n");
    
    /*
        ===============================================================================================================
        RECOMMENDED STARTUP SEQUENCE BEGIN
        ===============================================================================================================
    */

    result = FMOD_System_Create(&gSystem);
    ERRCHECK(result);
    
    result = FMOD_System_GetVersion(gSystem, &version);
    ERRCHECK(result);

    if (version < FMOD_VERSION)
    {
        printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
        return 0;
    }
    
    result = FMOD_System_GetNumDrivers(gSystem, &numdrivers);
    ERRCHECK(result);

    if (numdrivers == 0)
    {
        result = FMOD_System_SetOutput(gSystem, FMOD_OUTPUTTYPE_NOSOUND);
        ERRCHECK(result);
    }
    else
    {
        FMOD_CAPS caps;
        FMOD_SPEAKERMODE speakermode;
        char name[256];
        
        result = FMOD_System_GetDriverCaps(gSystem, 0, &caps, 0, &speakermode);
        ERRCHECK(result);

        result = FMOD_System_SetSpeakerMode(gSystem, speakermode);       /* Set the user selected speaker mode. */
        ERRCHECK(result);

        if (caps & FMOD_CAPS_HARDWARE_EMULATED)             /* The user has the 'Acceleration' slider set to off!  This is really bad for latency!. */
        {                                                   /* You might want to warn the user about this. */
            result = FMOD_System_SetDSPBufferSize(gSystem, 1024, 10);
            ERRCHECK(result);
        }

        result = FMOD_System_GetDriverInfo(gSystem, 0, name, 256, 0);
        ERRCHECK(result);

        if (strstr(name, "SigmaTel"))   /* Sigmatel sound devices crackle for some reason if the format is PCM 16bit.  PCM floating point output seems to solve it. */
        {
            result = FMOD_System_SetSoftwareFormat(gSystem, 48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0,0, FMOD_DSP_RESAMPLER_LINEAR);
            ERRCHECK(result);
        }
    }

    result = FMOD_System_Init(gSystem, 100, FMOD_INIT_NORMAL, 0);
    if (result == FMOD_ERR_OUTPUT_CREATEBUFFER)         /* Ok, the speaker mode selected isn't supported by this soundcard.  Switch it back to stereo... */
    {
        result = FMOD_System_SetSpeakerMode(gSystem, FMOD_SPEAKERMODE_STEREO);
        ERRCHECK(result);
            
        result = FMOD_System_Init(gSystem, 100, FMOD_INIT_NORMAL, 0);/* ... and re-init. */
        ERRCHECK(result);
    }
    
    /*
        ===============================================================================================================
        RECOMMENDED STARTUP SEQUENCE END
        ===============================================================================================================
    */
        
    result = FMOD_System_GetSoftwareFormat(gSystem, &outputrate, 0,0,0,0,0);
    ERRCHECK(result);   
   
#if !defined(USE_STREAMS)
    for (count = 0; count < NUMSOUNDS; count++)
    {
        result = FMOD_System_CreateSound(gSystem, soundname[count], FMOD_IGNORETAGS, 0, &sound[count]);
        ERRCHECK(result);
    }
#endif

    /*
        Kick off the first 2 sounds.  First one is immediate, second one will be triggered to start after the first one.
    */
    channel[slot] = queue_next_sound(outputrate, channel[1-slot], rand()%NUMSOUNDS, slot);
    slot = 1-slot;  /* flip */
    channel[slot] = queue_next_sound(outputrate, channel[1-slot], rand()%NUMSOUNDS, slot);
    slot = 1-slot;  /* flip */

    /*
        Main loop.
    */
    do
    {
        int isplaying;
        static int paused = 0;

        if (_kbhit())
        {
            key = _getch();

            switch (key)
            {
                case ' ' :
                {
                    set_paused(channel, paused);
                    paused = !paused;
                    break;
                }
            }
        }

        FMOD_System_Update(gSystem);

        /*
            Replace the sound that just finished with a new sound, to create endless seamless stitching!
        */
        result = FMOD_Channel_IsPlaying(channel[slot], &isplaying);
        if (!isplaying && !paused)
        {
            printf("sound %d finished, start a new one\n", slot);
            #ifdef USE_STREAMS
            /* 
                Release the sound that isn't playing any more. 
            */
            result = FMOD_Sound_Release(sound[slot]);       
            ERRCHECK(result);
            sound[slot] = 0;
            #endif
   
            /*
                Replace sound that just ended with a new sound, queued up to trigger exactly after the other sound ends.
            */
            channel[slot] = queue_next_sound(outputrate, channel[1-slot], rand()%NUMSOUNDS, slot);
            slot = 1-slot;  /* flip */
        }
        
        Sleep(10);  /* If you wait too long, ie longer than the length of the shortest sound, you will get gaps. */

    } while (key != 27);

    printf("\n");

    for (count = 0; count < sizeof(sound) / sizeof(sound[0]); count++)
    {
        if (sound[count])
        {
            FMOD_Sound_Release(sound[count]);
        }
    }
    
    /*
        Shut down
    */
    result = FMOD_System_Release(gSystem);
    ERRCHECK(result);

    return 0;
}
