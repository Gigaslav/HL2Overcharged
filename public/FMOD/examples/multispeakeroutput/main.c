/*===============================================================================================
 Multi Speaker Output Example
 Copyright (c), Firelight Technologies Pty, Ltd 2004-2016.

 This example shows how to play sounds in multiple speakers, and also how to even assign 
 sound subchannels, such as those in a stereo sound to different individual speakers.
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


int main(int argc, char *argv[])
{
    FMOD_SYSTEM      *system;
    FMOD_SOUND       *sound1, *sound2;
    FMOD_CHANNEL     *channel = 0;
    FMOD_RESULT       result;
    FMOD_SPEAKERMODE  speakermode;
    int               key;
    unsigned int      version;

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
       Choose the speaker mode selected by the Windows control panel.
    */
    result = FMOD_System_GetDriverCaps(system, 0, 0, 0, &speakermode);
    ERRCHECK(result);

    result = FMOD_System_SetSpeakerMode(system, speakermode);
    ERRCHECK(result);

    result = FMOD_System_Init(system, 32, FMOD_INIT_NORMAL, NULL);
    ERRCHECK(result);

    result = FMOD_System_CreateSound(system, "../media/drumloop.wav", FMOD_SOFTWARE | FMOD_2D, 0, &sound1);
    ERRCHECK(result);
    result = FMOD_Sound_SetMode(sound1, FMOD_LOOP_OFF);
    ERRCHECK(result);

    result = FMOD_System_CreateSound(system, "../media/stereo.ogg", FMOD_SOFTWARE | FMOD_2D, 0,  &sound2);
    ERRCHECK(result);

    printf("==============================================================================\n");
    printf("Multi Speaker Output Example.  Copyright (c) Firelight Technologies 2004-2016.\n");
    printf("==============================================================================\n");
    printf("\n");
    switch (speakermode)
    {
        case FMOD_SPEAKERMODE_MONO    :
        {
            printf("Using control panel speaker mode : MONO.\n");
            printf("\n");
            printf("Note! This output mode is very limited in its capability.\n");
            printf("Most functionality of this demo is only realized with at least FMOD_SPEAKERMODE_QUAD\n");
            printf("and above.\n");
            break;
        }
        case FMOD_SPEAKERMODE_STEREO  :
        {
            printf("Using control panel speaker mode : STEREO.\n");
            printf("\n");
            printf("Note! This output mode is very limited in its capability.\n");
            printf("Most functionality of this demo is only realized with FMOD_SPEAKERMODE_QUAD\n");
            printf("and above.\n");
            break;
        }
        case FMOD_SPEAKERMODE_QUAD :
        {
            printf("Using control panel speaker mode : QUAD.\n");
            printf("Side left, side right, center and subwoofer mix will be disabled.\n");
            break;
        }
        case FMOD_SPEAKERMODE_SURROUND :
        {
            printf("Using control panel speaker mode : SURROUND.\n");
            printf("Side left, side right, and subwoofer mix will be disabled.\n");
            break;
        }
        case FMOD_SPEAKERMODE_5POINT1 :
        {
            printf("Using control panel speaker mode : 5.1 surround.\n");
            printf("Side left and right mix will be disabled..\n");
            break;
        }
        case FMOD_SPEAKERMODE_7POINT1 :
        {
            printf("Using control panel speaker mode : 7.1 surround.\n");
            printf("Full capability.\n");
            break;
        }
    };
    printf("\n");

    printf("Press '1' to play a mono sound on the FRONT LEFT speaker.\n");
    printf("Press '2' to play a mono sound on the FRONT RIGHT speaker.\n");

    if (speakermode >= FMOD_SPEAKERMODE_SURROUND)
    {
        printf("Press '3' to play a mono sound on the CENTER speaker.\n");
    }
    else
    {
        printf("- CENTER Disabled\n");
    }

    if (speakermode >= FMOD_SPEAKERMODE_QUAD)
    {
        printf("Press '4' to play a mono sound on the REAR LEFT speaker.\n");
        printf("Press '5' to play a mono sound on the REAR RIGHT speaker.\n");
    }
    else
    {
        printf("- REAR LEFT Disabled\n");
        printf("- REAR RIGHT Disabled\n");
    }
    if (speakermode >= FMOD_SPEAKERMODE_7POINT1)
    {
        printf("Press '6' to play a mono sound on the SIDE LEFT speaker.\n");
        printf("Press '7' to play a mono sound on the SIDE RIGHT speaker.\n");
    }
    else
    {
        printf("- SIDE LEFT Disabled\n");
        printf("- SIDE RIGHT Disabled\n");
    }

    printf("\n");
    printf("Press '8' to play a stereo sound on the front speakers.\n");
    printf("Press '9' to play a stereo sound on the front speakers but channel swapped.\n");

    if (speakermode >= FMOD_SPEAKERMODE_SURROUND)
    {
        printf("Press '0' to play the right part of a stereo sound on the CENTER speaker.\n");
    }

    printf("Press 'Esc' to quit\n");
    printf("\n");

    /*
        Main loop.
    */
    do
    {
        if (_kbhit())
        {
            key = _getch();

            switch (key)
            {
                case '1' :
                {
                    result = FMOD_System_PlaySound(system, FMOD_CHANNEL_FREE, sound1, TRUE, &channel);
                    ERRCHECK(result);

                    result = FMOD_Channel_SetSpeakerMix(channel, 1.0f, 0, 0, 0, 0, 0, 0, 0);
                    ERRCHECK(result);

                    result = FMOD_Channel_SetPaused(channel, FALSE);
                    ERRCHECK(result);
                    break;
                }
                case '2' :
                {
                    result = FMOD_System_PlaySound(system, FMOD_CHANNEL_FREE, sound1, TRUE, &channel);
                    ERRCHECK(result);

                    result = FMOD_Channel_SetSpeakerMix(channel, 0, 1.0f, 0, 0, 0, 0, 0, 0);
                    ERRCHECK(result);

                    result = FMOD_Channel_SetPaused(channel, FALSE);
                    ERRCHECK(result);
                    break;
                }
                case '3' :
                {
                    if (speakermode >= FMOD_SPEAKERMODE_QUAD)
                    {
                        result = FMOD_System_PlaySound(system, FMOD_CHANNEL_FREE, sound1, TRUE, &channel);
                        ERRCHECK(result);

                        result = FMOD_Channel_SetSpeakerMix(channel, 0, 0, 1.0f, 0, 0, 0, 0, 0);
                        ERRCHECK(result);

                        result = FMOD_Channel_SetPaused(channel, FALSE);
                        ERRCHECK(result);
                    }
                    break;
                }
                case '4' :
                {
                    if (speakermode >= FMOD_SPEAKERMODE_QUAD)
                    {
                        result = FMOD_System_PlaySound(system, FMOD_CHANNEL_FREE, sound1, TRUE, &channel);
                        ERRCHECK(result);

                        result = FMOD_Channel_SetSpeakerMix(channel, 0, 0, 0, 0, 1.0f, 0, 0, 0);
                        ERRCHECK(result);

                        result = FMOD_Channel_SetPaused(channel, FALSE);
                        ERRCHECK(result);
                    }
                    break;
                }
                case '5' :
                {
                    if (speakermode >= FMOD_SPEAKERMODE_QUAD)
                    {
                        result = FMOD_System_PlaySound(system, FMOD_CHANNEL_FREE, sound1, TRUE, &channel);
                        ERRCHECK(result);

                        result = FMOD_Channel_SetSpeakerMix(channel, 0, 0, 0, 0, 0, 1.0f, 0, 0);
                        ERRCHECK(result);

                        result = FMOD_Channel_SetPaused(channel, FALSE);
                        ERRCHECK(result);
                    }
                    break;
                }
                case '6' :
                {
                    if (speakermode >= FMOD_SPEAKERMODE_7POINT1)
                    {
                        result = FMOD_System_PlaySound(system, FMOD_CHANNEL_FREE, sound1, TRUE, &channel);
                        ERRCHECK(result);

                        result = FMOD_Channel_SetSpeakerMix(channel, 0, 0, 0, 0, 0, 0, 1.0f, 0);
                        ERRCHECK(result);

                        result = FMOD_Channel_SetPaused(channel, FALSE);
                        ERRCHECK(result);
                    }
                    break;
                }
                case '7' :
                {
                    if (speakermode >= FMOD_SPEAKERMODE_7POINT1)
                    {
                        result = FMOD_System_PlaySound(system, FMOD_CHANNEL_FREE, sound1, TRUE, &channel);
                        ERRCHECK(result);

                        result = FMOD_Channel_SetSpeakerMix(channel, 0, 0, 0, 0, 0, 0, 0, 1.0f);
                        ERRCHECK(result);

                        result = FMOD_Channel_SetPaused(channel, FALSE);
                        ERRCHECK(result);
                    }
                    break;
                }
                case '8' :
                {
                    float  levels[2] = { 0, 1.0f };

                    result = FMOD_System_PlaySound(system, FMOD_CHANNEL_FREE, sound2, TRUE, &channel);
                    ERRCHECK(result);

                    /*
                        By default a stereo sound would play in all right and all left speakers, so this forces it to just the front.
                    */
                    result = FMOD_Channel_SetSpeakerMix(channel, 1.0f, 1.0f, 0, 0, 0, 0, 0, 0);
                    ERRCHECK(result);

                    result = FMOD_Channel_SetPaused(channel, FALSE);
                    ERRCHECK(result);

                    break;
                }
                case '9' :
                {
                    result = FMOD_System_PlaySound(system, FMOD_CHANNEL_FREE, sound2, TRUE, &channel);
                    ERRCHECK(result);

                    /*
                        Clear out all speakers first.
                    */
                    result = FMOD_Channel_SetSpeakerMix(channel, 0, 0, 0, 0, 0, 0, 0, 0);
                    ERRCHECK(result);

                    /*
                        Put the left channel of the sound in the right speaker.
                    */
                    {
                        float  levels[2] = { 0, 1.0f };    /* This array represents the source stereo sound.  l/r */

                        result = FMOD_Channel_SetSpeakerLevels(channel, FMOD_SPEAKER_FRONT_LEFT, levels, 2);
                        ERRCHECK(result);
                    }
                    /*
                        Put the right channel of the sound in the left speaker.
                    */
                    {
                        float  levels[2] = { 1.0f, 0 };    /* This array represents the source stereo sound.  l/r */

                        result = FMOD_Channel_SetSpeakerLevels(channel, FMOD_SPEAKER_FRONT_RIGHT, levels, 2);
                        ERRCHECK(result);
                    }

                    result = FMOD_Channel_SetPaused(channel, FALSE);
                    ERRCHECK(result);

                    break;
                }
                case '0' :
                {
                    if (speakermode >= FMOD_SPEAKERMODE_SURROUND)   /* All formats that have a center speaker. */
                    {
                        result = FMOD_System_PlaySound(system, FMOD_CHANNEL_FREE, sound2, TRUE, &channel);
                        ERRCHECK(result);

                        /*
                            Clear out all speakers first.
                        */
                        result = FMOD_Channel_SetSpeakerMix(channel, 0, 0, 0, 0, 0, 0, 0, 0);
                        ERRCHECK(result);

                        /*
                            Put the right channel of the sound in the center speaker.
                        */
                        {
                            float  levels[2] = { 0, 1.0f };    /* This array represents the source stereo sound.  l/r */

                            result = FMOD_Channel_SetSpeakerLevels(channel, FMOD_SPEAKER_FRONT_CENTER, levels, 2);
                            ERRCHECK(result);
                        }

                        result = FMOD_Channel_SetPaused(channel, FALSE);
                        ERRCHECK(result);
                    }
                    break;
                }
            }
        }

        FMOD_System_Update(system);

        {
            unsigned int ms = 0;
            unsigned int lenms = 0;
            int          playing = FALSE;
            int          paused = FALSE;
            int          channelsplaying = 0;

            if (channel)
            {
                FMOD_SOUND *currentsound = 0;

                result = FMOD_Channel_IsPlaying(channel, &playing);
                if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
                {
                    ERRCHECK(result);
                }

                result = FMOD_Channel_GetPaused(channel, &paused);
                if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
                {
                    ERRCHECK(result);
                }

                result = FMOD_Channel_GetPosition(channel, &ms, FMOD_TIMEUNIT_MS);
                if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
                {
                    ERRCHECK(result);
                }
               
                FMOD_Channel_GetCurrentSound(channel, &currentsound);
                if (currentsound)
                {
                    result = FMOD_Sound_GetLength(currentsound, &lenms, FMOD_TIMEUNIT_MS);
                    if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
                    {
                        ERRCHECK(result);
                    }
                }
            }

            FMOD_System_GetChannelsPlaying(system, &channelsplaying);

            printf("Time %02d:%02d:%02d/%02d:%02d:%02d : %s : Channels Playing %2d\r", ms / 1000 / 60, ms / 1000 % 60, ms / 10 % 100, lenms / 1000 / 60, lenms / 1000 % 60, lenms / 10 % 100, paused ? "Paused " : playing ? "Playing" : "Stopped", channelsplaying);
        }

        Sleep(10);

    } while (key != 27);

    printf("\n");

    /*
        Shut down
    */
    result = FMOD_Sound_Release(sound1);
    ERRCHECK(result);
    result = FMOD_Sound_Release(sound2);
    ERRCHECK(result);
    result = FMOD_System_Close(system);
    ERRCHECK(result);
    result = FMOD_System_Release(system);
    ERRCHECK(result);

    return 0;
}


