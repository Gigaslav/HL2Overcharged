/*===============================================================================================
 Use Plugins Example
 Copyright (c), Firelight Technologies Pty, Ltd 2004-2016.

 This example shows how to use FMODEXP.DLL and its associated plugins.
 The example lists the available plugins, and loads a new plugin that isnt normally included
 with FMOD Ex, which is output_mp3.dll.  When this is loaded, it can be chosen as an output
 mode, for realtime encoding of audio output into the mp3 format.
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
    FMOD_SYSTEM     *system  = 0;
    FMOD_SOUND      *sound   = 0;
    FMOD_CHANNEL    *channel = 0;
    FMOD_RESULT      result;
    int              count;
    int              playing = FALSE;
    int              key, numoutputplugins;
    unsigned int     version;
    unsigned int     handle;

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
        Set the source directory for all of the FMOD plugins.
    */
    result = FMOD_System_SetPluginPath(system, "../../api/plugins");
    ERRCHECK(result);

    /*
        Load up an extra plugin that is not normally used by FMOD.
    */
    result = FMOD_System_LoadPlugin(system, "output_mp3.dll", 0, 0);
    if (result == FMOD_ERR_FILE_NOTFOUND)
    {
        /*
            If it isn't in the same directory, try for the plugin directory.
        */
        result = FMOD_System_LoadPlugin(system, "../plugin_dev/output_mp3/output_mp3.dll", 0, 0);
        ERRCHECK(result);
    }


    /*
        Display plugins
    */
    {
        int num;
        char name[256];

        printf("Codec plugins\n");
        printf("--------------\n");
        result = FMOD_System_GetNumPlugins(system, FMOD_PLUGINTYPE_CODEC, &num);
        ERRCHECK(result);
        for (count = 0; count < num; count++)
        {
            result = FMOD_System_GetPluginHandle(system, FMOD_PLUGINTYPE_CODEC, count, &handle);
            ERRCHECK(result);
            
            result = FMOD_System_GetPluginInfo(system, handle, 0, name, 256, 0);
            ERRCHECK(result);

            printf("%2d - %-30s", count + 1, name);

            if (count % 2)
            {
                printf("\n");
            }
        }
        printf("\n");
        if (count % 2)
        {
            printf("\n");
        }

        printf("DSP plugins\n");
        printf("--------------\n");
        result = FMOD_System_GetNumPlugins(system, FMOD_PLUGINTYPE_DSP, &num);
        ERRCHECK(result);
        for (count = 0; count < num; count++)
        {
            result = FMOD_System_GetPluginHandle(system, FMOD_PLUGINTYPE_DSP, count, &handle);
            ERRCHECK(result);

            result = FMOD_System_GetPluginInfo(system, handle, 0, name, 256, 0);
            ERRCHECK(result);

            printf("%2d - %-30s", count + 1, name);

            if (count % 2)
            {
                printf("\n");
            }
        }
        printf("\n");
        if (count % 2)
        {
            printf("\n");
        }

        printf("Output plugins\n");
        printf("--------------\n");
        result = FMOD_System_GetNumPlugins(system, FMOD_PLUGINTYPE_OUTPUT, &numoutputplugins);
        ERRCHECK(result);
        for (count = 0; count < numoutputplugins; count++)
        {
            result = FMOD_System_GetPluginHandle(system, FMOD_PLUGINTYPE_OUTPUT, count, &handle);
            ERRCHECK(result);

            result = FMOD_System_GetPluginInfo(system, handle, 0, name, 256, 0);
            ERRCHECK(result);

            printf("%2d - %-30s", count + 1, name);

            if (count % 2)
            {
                printf("\n");
            }
        }
        if (count % 2)
        {
            printf("\n");
        }
    }

    /*
        System initialization
    */
    printf("-----------------------------------------------------------------------\n");    // print driver names
    printf("Press a corresponding number for an OUTPUT PLUGIN to use or ESC to quit\n");

    do
    {
        key = _getch();
    } while (key != 27 && key < '1' && key > '0' + numoutputplugins);
    if (key == 27)
    {
        return 0;
    }
    
    result = FMOD_System_GetPluginHandle(system, FMOD_PLUGINTYPE_DSP, key - '1', &handle);
    ERRCHECK(result);

    result = FMOD_System_SetOutputByPlugin(system, handle);
    ERRCHECK(result);

    result = FMOD_System_Init(system, 32, FMOD_INIT_NORMAL, NULL);
    ERRCHECK(result);

    result = FMOD_System_CreateSound(system, "../media/wave.mp3", FMOD_SOFTWARE | FMOD_CREATESTREAM, 0, &sound);
    ERRCHECK(result);

    printf("Press a key to play sound to output device.\n");

    result = FMOD_System_PlaySound(system, FMOD_CHANNEL_FREE, sound, 0, &channel);
    ERRCHECK(result);

    /*
        Main loop.
    */
    do
    {
        unsigned int ms = 0;
        unsigned int lenms = 0;
        int          paused = FALSE;
        int          channelsplaying = 0;
        FMOD_SOUND  *currentsound = 0;

        FMOD_System_Update(system);

        playing = FALSE;

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

        FMOD_System_GetChannelsPlaying(system, &channelsplaying);

        printf("Time %02d:%02d:%02d/%02d:%02d:%02d : %s : Channels Playing %2d\r", ms / 1000 / 60, ms / 1000 % 60, ms / 10 % 100, lenms / 1000 / 60, lenms / 1000 % 60, lenms / 10 % 100, paused ? "Paused " : playing ? "Playing" : "Stopped", channelsplaying);

        Sleep(5);

        if (_kbhit())
        {
            key = _getch();
            if (key == 27)
            {
                break;
            }
        }

    } while (playing);

    printf("\n");

    /*
        Shut down
    */
    result = FMOD_Sound_Release(sound);
    ERRCHECK(result);
    result = FMOD_System_Close(system);
    ERRCHECK(result);
    result = FMOD_System_Release(system);
    ERRCHECK(result);

    return 0;
}


