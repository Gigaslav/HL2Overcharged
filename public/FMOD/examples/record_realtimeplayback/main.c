/*===============================================================================================
 Record to disk example
 Copyright (c), Firelight Technologies Pty, Ltd 2004-2016.

 This example shows how to record continuously, and play back the same data as closely to 
 the record cursor as possible without stuttering.
 
 NOTE - This setup is about 250ms latency to be 'safe', because combinations of operating system
 and sound card driver can cause stuttering if the settings are set too low.  To reduce latency
 on playback, use #define LOWLATENCY.   Currently it is commented out.
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

#define LOWLATENCY      /* Only really suited to Windows Vista/7.  XP/Directsound should avoid this. */

#define RECORDRATE      8000

#ifdef LOWLATENCY
    #define LATENCY         ((RECORDRATE * 20) / 1000)   /* 20 = 20ms */
    #define DRIFTTHRESHOLD  ((RECORDRATE * 10) / 1000)   /* 10 = 10ms */
#else
    #define LATENCY         ((RECORDRATE * 50) / 1000)   /* 50 = 50ms */
    #define DRIFTTHRESHOLD  ((RECORDRATE * 25) / 1000)   /* 25 = 25ms */
#endif

int main(int argc, char *argv[])
{
    FMOD_SYSTEM           *system  = 0;
    FMOD_SOUND            *sound   = 0;
    FMOD_CHANNEL          *channel = 0;
    FMOD_DSP              *dsp     = 0;
    FMOD_RESULT            result;
    FMOD_CREATESOUNDEXINFO exinfo;
    FMOD_SPEAKERMODE       speakermode;
    FMOD_CAPS              caps;
    int                    key, numdrivers;
    unsigned int           version;    
    unsigned int           datalength = 0, soundlength;
    char                   name[256];
    unsigned int           adjustedlatency;

    /*
        Create a System object and initialize.
    */
    result = FMOD_System_Create(&system);
    ERRCHECK(result);

    result = FMOD_System_GetVersion(system, &version);
    ERRCHECK(result);

    if (version < FMOD_VERSION)
    {
        printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
        return 0;
    }

    /* 
        System initialization (recommended startup sequence)
    */
    result = FMOD_System_GetNumDrivers(system, &numdrivers);
    ERRCHECK(result);

    if (numdrivers == 0)
    {
        result = FMOD_System_SetOutput(system, FMOD_OUTPUTTYPE_NOSOUND);
        ERRCHECK(result);
    }
    else
    {
        result = FMOD_System_GetDriverCaps(system, 0, &caps, 0, &speakermode);
        ERRCHECK(result);

        result = FMOD_System_SetSpeakerMode(system, speakermode);       /* Set the user selected speaker mode. */
        ERRCHECK(result);

        if (caps & FMOD_CAPS_HARDWARE_EMULATED)             /* The user has the 'Acceleration' slider set to off!  This is really bad for latency!. */
        {                                                   /* You might want to warn the user about this. */
            result = FMOD_System_SetDSPBufferSize(system, 1024, 10);
            ERRCHECK(result);
        }
#ifdef LOWLATENCY
        else
        {
            result = FMOD_System_SetDSPBufferSize(system, 256, 4);
        }
#endif
        result = FMOD_System_GetDriverInfo(system, 0, name, 256, 0);
        ERRCHECK(result);

        if (strstr(name, "SigmaTel"))   /* Sigmatel sound devices crackle for some reason if the format is PCM 16bit.  PCM floating point output seems to solve it. */
        {
            result = FMOD_System_SetSoftwareFormat(system, 48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0,0, FMOD_DSP_RESAMPLER_LINEAR);
            ERRCHECK(result);
        }
    }

    result = FMOD_System_Init(system, 100, FMOD_INIT_NORMAL, 0);
    if (result == FMOD_ERR_OUTPUT_CREATEBUFFER)         /* Ok, the speaker mode selected isn't supported by this soundcard.  Switch it back to stereo... */
    {
        result = FMOD_System_SetSpeakerMode(system, FMOD_SPEAKERMODE_STEREO);
        ERRCHECK(result);
            
        result = FMOD_System_Init(system, 100, FMOD_INIT_NORMAL, 0);/* ... and re-init. */
        ERRCHECK(result);
    }
    /* 
        System initialization complete (recommended startup sequence)
    */
 

    /*
        Create user sound to record into.  Set it to loop for playback.
    */
    memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    exinfo.cbsize           = sizeof(FMOD_CREATESOUNDEXINFO);
    exinfo.numchannels      = 1;
    exinfo.format           = FMOD_SOUND_FORMAT_PCM16;
    exinfo.defaultfrequency = RECORDRATE;
    exinfo.length           = exinfo.defaultfrequency * sizeof(short) * exinfo.numchannels * 5; /* 5 second buffer, doesnt really matter how big this is, but not too small of course. */
    
    result = FMOD_System_CreateSound(system, 0, FMOD_2D | FMOD_SOFTWARE | FMOD_LOOP_NORMAL | FMOD_OPENUSER, &exinfo, &sound);
    ERRCHECK(result);

    printf("========================================================================\n");
    printf("Record with realtime playback example.\n");
    printf("Copyright (c) Firelight Technologies 2004-2016.\n");
    printf("\n");
    printf("Try #define LOWLATENCY to reduce latency for more modern machines!\n");
    printf("========================================================================\n");
    printf("\n");
    printf("Press a key to start recording.  Playback will start %d samples (%d ms) later.\n", LATENCY, LATENCY * 1000 / RECORDRATE);
    printf("\n");

    _getch();

    printf("Press 'E' to toggle an effect on/off.\n");
    printf("Press 'Esc' to quit\n");
    printf("\n");

    result = FMOD_System_RecordStart(system, 0, sound, TRUE);
    ERRCHECK(result);

    result = FMOD_Sound_GetLength(sound, &soundlength, FMOD_TIMEUNIT_PCM);
    ERRCHECK(result);

    /*
        Create a DSP effect to play with.
    */
    result = FMOD_System_CreateDSPByType(system, FMOD_DSP_TYPE_FLANGE, &dsp);
    ERRCHECK(result);
    result = FMOD_DSP_SetParameter(dsp, FMOD_DSP_FLANGE_RATE, 4.0f);
    ERRCHECK(result);
    result = FMOD_DSP_SetBypass(dsp, TRUE);   
    ERRCHECK(result);
    
    adjustedlatency = LATENCY;  /* This might change depending on record block size. */
    
    /*
        Main loop.
    */
    do
    {
        static unsigned int lastrecordpos = 0, samplesrecorded = 0;
        unsigned int recordpos = 0, recorddelta;
        
        key = 0;
        if (_kbhit())
        {
            key = _getch();
        }

        if (key == 'e' || key == 'E')
        {
            int bypass;
            FMOD_DSP_GetBypass(dsp, &bypass);
            FMOD_DSP_SetBypass(dsp, !bypass);
            if (bypass)
            {
                FMOD_REVERB_PROPERTIES prop = FMOD_PRESET_CONCERTHALL;
                FMOD_System_SetReverbProperties(system, &prop);
            }
            else
            {
                FMOD_REVERB_PROPERTIES prop = FMOD_PRESET_OFF;
                FMOD_System_SetReverbProperties(system, &prop);
            }
            printf("\n\n *** TURN DSP EFFECT %s ** \n\n", bypass ? "ON" : "OFF");
        }
        
        FMOD_System_GetRecordPosition(system, 0, &recordpos);
        ERRCHECK(result);

        recorddelta = recordpos >= lastrecordpos ? recordpos - lastrecordpos : recordpos + soundlength - lastrecordpos;
        samplesrecorded += recorddelta;

		if (samplesrecorded >= adjustedlatency && !channel)
		{
		    result = FMOD_System_PlaySound(system, FMOD_CHANNEL_FREE, sound, 0, &channel);
            ERRCHECK(result);
            
            result = FMOD_Channel_AddDSP(channel, dsp, 0);
            ERRCHECK(result);
		}
				
        if (channel && recorddelta)
        {
            unsigned int playrecorddelta;
            unsigned int playpos = 0;
            int adjusting = 0;
            float smootheddelta;
            float dampratio = 0.97f;
            static unsigned int minrecorddelta = (unsigned int)-1;
            
            /*
                If the record driver steps the position of the record cursor in larger increments than the user defined latency value, then we should
                increase our latency value to match.
            */
            if (recorddelta < minrecorddelta)
            {
                minrecorddelta = recorddelta;
                if (adjustedlatency < recorddelta)
                {
                    adjustedlatency = recorddelta;
                }
            }

            FMOD_Channel_GetPosition(channel, &playpos, FMOD_TIMEUNIT_PCM);

            playrecorddelta = recordpos >= playpos ? recordpos - playpos : recordpos + soundlength - playpos;
            
	        /*
                Smooth total
            */
            {
                static float total = 0;
                
                total = total * dampratio;
	            total += playrecorddelta;
	            smootheddelta = total * (1.0f - dampratio);
            }
           
            if (smootheddelta < adjustedlatency - DRIFTTHRESHOLD || smootheddelta > soundlength / 2)   /* if play cursor is catching up to record (or passed it), slow playback down */
            {
                FMOD_Channel_SetFrequency(channel, RECORDRATE - (RECORDRATE / 50)); /* Decrease speed by 2% */
                adjusting = 1;
            }
            else if (smootheddelta > adjustedlatency + DRIFTTHRESHOLD)   /* if play cursor is falling too far behind record, speed playback up */
            {
                FMOD_Channel_SetFrequency(channel, RECORDRATE + (RECORDRATE / 50)); /* Increase speed by 2% */
                adjusting = 2;
            }
            else
            {
                FMOD_Channel_SetFrequency(channel, RECORDRATE);          /* Otherwise set to normal rate */
                adjusting = 0;
            }
            
            printf("REC %5d (REC delta %5d) : PLAY %5d, PLAY/REC diff %5d %s\r", recordpos, recorddelta, playpos, (int)smootheddelta, adjusting == 1 ? "DECREASE SPEED" : adjusting == 2 ? "INCREASE SPEED" : "              ");
        }
        
        lastrecordpos = recordpos;
        
        FMOD_System_Update(system);

        Sleep(10);

    } while (key != 27);

    printf("\n");

    /*
        Shut down
    */
    result = FMOD_Sound_Release(sound);
    ERRCHECK(result);

    result = FMOD_System_Release(system);
    ERRCHECK(result);

    return 0;
}
