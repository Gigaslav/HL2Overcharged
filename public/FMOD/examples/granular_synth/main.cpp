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

#include "../../api/inc/fmod.hpp"
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

FMOD::System *gSystem;

#ifdef USE_STREAMS
    #define NUMSOUNDS 3                         /* Use some longer sounds, free and load them on the fly. */
    FMOD::Sound      *sound[2] = { 0,0 };       /* 2 streams active, double buffer them. */
    const char       *soundname[NUMSOUNDS] = 
    {
        "../media/c.ogg",
        "../media/d.ogg",
        "../media/e.ogg",
    };
#else
    #define NUMSOUNDS 6                                     /* These sounds will be loaded into memory statically. */
    FMOD::Sound      *sound[NUMSOUNDS] = { 0,0,0,0,0,0 };   /* 6 sounds active, one for each wav. */
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

void set_paused(FMOD::Channel **channel, bool paused)
{
    FMOD_RESULT result;
    unsigned int blocksize;
    
    result = gSystem->getDSPBufferSize(&blocksize, 0);
    ERRCHECK(result);

    if (!paused)
    {
        unsigned int pausestart_hi = 0, pausestart_lo = 0;

        gSystem->getDSPClock(&pausestart_hi, &pausestart_lo);
        
        FMOD_64BIT_ADD(pausestart_hi, pausestart_lo, 0, blocksize * 2);   /* Into the future by 2 mixer blocks. */
        printf("\npause BOTH at %d \n", pausestart_lo);
       
        /* Make them both pause at exactly the same tick.  Mute them both to avoid a click as well. */
        channel[0]->setMute(true);
        channel[0]->setDelay(FMOD_DELAYTYPE_DSPCLOCK_PAUSE, pausestart_hi, pausestart_lo);
        channel[1]->setMute(true);
        channel[1]->setDelay(FMOD_DELAYTYPE_DSPCLOCK_PAUSE, pausestart_hi, pausestart_lo);
    }
    else
    {
        unsigned int syshi, syslo;
        int count;

        gSystem->getDSPClock(&syshi, &syslo);

        printf("\nunpause BOTH at %d\n", syslo);
        
        for (count = 0; count < 2; count++)
        { 
            unsigned int starttime_hi, starttime_lo; 
            unsigned int pausetime_hi = 0, pausetime_lo = 0;
            unsigned int hi = syshi, lo = syslo;
            
            channel[count]->getDelay(FMOD_DELAYTYPE_DSPCLOCK_PAUSE, &pausetime_hi, &pausetime_lo);
            channel[count]->getDelay(FMOD_DELAYTYPE_DSPCLOCK_START, &starttime_hi, &starttime_lo);

            FMOD_64BIT_ADD(hi, lo, 0, blocksize * 2);                   /* Push operation into the future by 2 mixer blocks so it doesnt conflict with mixer. */
            if (starttime_lo > pausetime_lo)                            /* Was already playing, unpause immediately. */
            {
                FMOD_64BIT_ADD(hi, lo, starttime_hi, starttime_lo);     /* Push forward the delayed start by the gap between starting and pausing */
                FMOD_64BIT_SUB(hi, lo, pausetime_hi, pausetime_lo);     /* Push forward the delayed start by the gap between starting and pausing */
            }
            printf("restart %d at %d\n", count, lo);
            channel[count]->setDelay(FMOD_DELAYTYPE_DSPCLOCK_PAUSE, 0, 0);
            channel[count]->setDelay(FMOD_DELAYTYPE_DSPCLOCK_START, hi, lo);
            channel[count]->setMute(false);
            channel[count]->setPaused(false);
        }
    }
}


FMOD::Channel *queue_next_sound(int outputrate, FMOD::Channel *playingchannel, int newindex, int slot)
{
    FMOD_RESULT result;
    FMOD::Channel *newchannel;
    FMOD::Sound *newsound;
    
    #ifdef USE_STREAMS                                  /* Create a new stream */
    FMOD_CREATESOUNDEXINFO info;
    memset(&info, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    info.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    info.suggestedsoundtype = FMOD_SOUND_TYPE_OGGVORBIS;
    result = gSystem->createStream(soundname[newindex], FMOD_IGNORETAGS | FMOD_LOWMEM, &info, &sound[slot]);
    ERRCHECK(result);
    newsound = sound[slot];
    #else                                               /* Use an existing sound that was passed into us */
    newsound = sound[newindex];
    #endif
    
    result = gSystem->playSound(FMOD_CHANNEL_FREE, newsound, true, &newchannel);
    ERRCHECK(result);
      
    result = newchannel->setSpeakerMix(1,1,1,1,1,1,1,1);
    ERRCHECK(result);
           
    
    if (playingchannel)
    {    
        unsigned int hi = 0, lo = 0, sound_length;
        float sound_frequency;
        FMOD::Sound *playingsound;
        
        /*
            Get the start time of the playing channel.
        */
        result = playingchannel->getDelay(FMOD_DELAYTYPE_DSPCLOCK_START, &hi, &lo);
        ERRCHECK(result);
        
        printf("playing sound started at %d\n", lo);
        
        /*
            Grab the length of the playing sound, and its frequency, so we can caluate where to place the new sound on the time line.
        */
        result = playingchannel->getCurrentSound(&playingsound);
        ERRCHECK(result);
        result = playingsound->getLength(&sound_length, FMOD_TIMEUNIT_PCM);
        ERRCHECK(result);
        result = playingchannel->getFrequency(&sound_frequency);
        ERRCHECK(result);
        
        /* 
            Now calculate the length of the sound in 'output samples'.  
            Ie if a 44khz sound is 22050 samples long, and the output rate is 48khz, then we want to delay by 24000 output samples.
        */
        sound_length *= outputrate;   
        sound_length /= (int)sound_frequency;
        
        FMOD_64BIT_ADD(hi, lo, 0, sound_length);  /* Add output rate adjusted sound length, to the clock value of the sound that is currently playing */
            
        result = newchannel->setDelay(FMOD_DELAYTYPE_DSPCLOCK_START, hi, lo);      /* Set the delay of the new sound to the end of the old sound */
        ERRCHECK(result);
    }
    
    {
        unsigned int hi = 0, lo = 0;
        float val, variation;
        
        /*
            Randomize pitch/volume to make it sound more realistic / random.
        */
        newchannel->getFrequency(&val);
        variation = (((float)(rand()%10000) / 5000.0f) - 1.0f); /* -1.0 to +1.0 */
        val *= (1.0f + (variation * 0.02f));                    /* @22khz, range fluctuates from 21509 to 22491 */
        newchannel->setFrequency(val);

        newchannel->getVolume(&val);
        variation = ((float)(rand()%10000) / 10000.0f);         /*  0.0 to 1.0 */
        val *= (1.0f - (variation * 0.2f));                     /*  0.8 to 1.0 */
        newchannel->setVolume(val);
        
        newchannel->getDelay(FMOD_DELAYTYPE_DSPCLOCK_START, &hi, &lo);
        printf("new sound to start at %d (slot %d)\n", lo, slot);
    }   
        
    result = newchannel->setPaused(false);
    ERRCHECK(result);
       
    return newchannel;
}

int main(int argc, char *argv[])
{
    FMOD::Channel    *channel[2] = { 0,0 };
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

    result = FMOD::System_Create(&gSystem);
    ERRCHECK(result);
    
    result = gSystem->getVersion(&version);
    ERRCHECK(result);

    if (version < FMOD_VERSION)
    {
        printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
        return 0;
    }
    
    result = gSystem->getNumDrivers(&numdrivers);
    ERRCHECK(result);

    if (numdrivers == 0)
    {
        result = gSystem->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
        ERRCHECK(result);
    }
    else
    {
        FMOD_CAPS caps;
        FMOD_SPEAKERMODE speakermode;
        char name[256];
        
        result = gSystem->getDriverCaps(0, &caps, 0, &speakermode);
        ERRCHECK(result);

        result = gSystem->setSpeakerMode(speakermode);       /* Set the user selected speaker mode. */
        ERRCHECK(result);

        if (caps & FMOD_CAPS_HARDWARE_EMULATED)             /* The user has the 'Acceleration' slider set to off!  This is really bad for latency!. */
        {                                                   /* You might want to warn the user about this. */
            result = gSystem->setDSPBufferSize(1024, 10);
            ERRCHECK(result);
        }

        result = gSystem->getDriverInfo(0, name, 256, 0);
        ERRCHECK(result);

        if (strstr(name, "SigmaTel"))   /* Sigmatel sound devices crackle for some reason if the format is PCM 16bit.  PCM floating point output seems to solve it. */
        {
            result = gSystem->setSoftwareFormat(48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0,0, FMOD_DSP_RESAMPLER_LINEAR);
            ERRCHECK(result);
        }
    }

    result = gSystem->init(100, FMOD_INIT_NORMAL, 0);
    if (result == FMOD_ERR_OUTPUT_CREATEBUFFER)         /* Ok, the speaker mode selected isn't supported by this soundcard.  Switch it back to stereo... */
    {
        result = gSystem->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
        ERRCHECK(result);
            
        result = gSystem->init(100, FMOD_INIT_NORMAL, 0);/* ... and re-init. */
        ERRCHECK(result);
    }
    
    /*
        ===============================================================================================================
        RECOMMENDED STARTUP SEQUENCE END
        ===============================================================================================================
    */
        
    result = gSystem->getSoftwareFormat(&outputrate, 0,0,0,0,0);
    ERRCHECK(result);   
   
#if !defined(USE_STREAMS)
    for (count = 0; count < NUMSOUNDS; count++)
    {
        result = gSystem->createSound(soundname[count], FMOD_IGNORETAGS, 0, &sound[count]);
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
        bool isplaying;
        static bool paused = false;

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

        gSystem->update();

        /*
            Replace the sound that just finished with a new sound, to create endless seamless stitching!
        */
        result = channel[slot]->isPlaying(&isplaying);
        if (!isplaying && !paused)
        {
            printf("sound %d finished, start a new one\n", slot);
            #ifdef USE_STREAMS
            /* 
                Release the sound that isn't playing any more. 
            */
            result = sound[slot]->release();       
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
            sound[count]->release();
        }
    }
    
    /*
        Shut down
    */
    result = gSystem->release();
    ERRCHECK(result);

    return 0;
}
