/*===============================================================================================
 Load From Memory Example
 Copyright (c), Firelight Technologies Pty, Ltd 2004-2016.

 Demonstrates loading all resources from memory allocated and filled by the user.
===============================================================================================*/
#include "../../api/inc/fmod_event.h"
#include "../../api/inc/fmod_errors.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }
}


void loadFileIntoMemory(const char *file_name, void **data, unsigned int *length)
{
    FILE *file = 0;
    unsigned int len = 0;
    unsigned int read = 0;
    void *mem = 0;

    printf("loading file: %s\n", file_name);
    file = fopen(file_name, "rb");
    if (!file)
    {
        printf("error opening file %s\n", file_name);
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    len =  ftell(file);
    mem = malloc(len);
    if (!mem)
    {
        printf("allocation failed\n");
        exit(1);
    }
    rewind(file);
    read = fread(mem, sizeof(char), len, file);
    if (read != len)
    {
        printf("error reading file\n");
        exit(1);
    }

    *data   = mem;
    *length = len;
}


int main(int argc, char *argv[])
{
    FMOD_EVENTSYSTEM    *eventsystem = 0;
    FMOD_SYSTEM         *system      = 0;

    void                 *project_mem = 0;
    unsigned int          project_len = 0;

    FMOD_EVENT           *sampleevent    = 0;
    FMOD_SOUND           *samplebank     = 0;
    void                 *samplebank_mem = 0;
    unsigned int          samplebank_len = 0;

    FMOD_EVENT           *streamevent = 0;
    FMOD_SOUND           *streambank  = 0;

    FMOD_RESULT           result = FMOD_OK;
    int                   key = 0;
    FMOD_EVENT_LOADINFO load_info;

    printf("======================================================================\n");
    printf("Load Event Data Example.  Copyright (c) Firelight Technologies 2004-2016.\n");
    printf("==============================-------=================================\n");
    printf("This Demonstrates loading all resources from memory allocated and filled\n");
    printf("by the user.\n");
    printf("======================================================================\n\n");
    
    /* Load FEV file into memory */
    loadFileIntoMemory("..\\media\\examples.fev", &project_mem, &project_len);

    ERRCHECK(result = FMOD_EventSystem_Create(&eventsystem));
    ERRCHECK(result = FMOD_EventSystem_GetSystemObject(eventsystem, &system));
    ERRCHECK(result = FMOD_EventSystem_Init(eventsystem, 64, FMOD_INIT_NORMAL, 0, FMOD_EVENT_INIT_NORMAL));
    ERRCHECK(result = FMOD_EventSystem_SetMediaPath(eventsystem, "..\\media\\"));

    /* we require a loadinfo struct to tell FMOD how big the in memory FEV file is */
    memset(&load_info, 0, sizeof(FMOD_EVENT_LOADINFO));
    load_info.size = sizeof(FMOD_EVENT_LOADINFO);
    load_info.loadfrommemory_length = project_len;

    /* load the project from memory */
    ERRCHECK(result = FMOD_EventSystem_Load(eventsystem, (char*)project_mem, &load_info, NULL));

    printf("======================================================================\n");
    printf("Press 'e'        to load sample data\n");
    printf("Press 'E'        to unload sample data\n");
    printf("Press 'd'        to start sample event\n");
    printf("Press 'w'        to open stream\n");
    printf("Press 'W'        to close stream\n");
    printf("Press 's'        to start stream event\n");
    printf("Press ESC        to quit\n");
    printf("======================================================================\n");

    key = 0;
    do
    {
        if (_kbhit())
        {
            key = _getch();

            if (key == 'e')
            {
                /* Attempt to get the event without disk access */
                result = FMOD_EventSystem_GetEvent(eventsystem, "examples/FeatureDemonstration/Basics/SimpleEvent", FMOD_EVENT_ERROR_ON_DISKACCESS, &sampleevent);
                if (result != FMOD_ERR_FILE_UNWANTED)
                {
                    ERRCHECK(result);
                }
                else
                {
                    /* file unwanted error tells us we haven't got the soundbank preloaded, so preload it now... */
                    FMOD_CREATESOUNDEXINFO info = {0};
                    printf("Loading event data\n");

                    loadFileIntoMemory("..\\media\\tutorial_bank.fsb", &samplebank_mem, &samplebank_len);

                    /* We need to create a FMOD_SOUND object to use with preloadFSB */
                    info.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
                    info.length = samplebank_len;
                    ERRCHECK(result = FMOD_System_CreateSound(system, (char*)samplebank_mem, FMOD_OPENMEMORY_POINT | FMOD_CREATECOMPRESSEDSAMPLE, &info, &samplebank));

                    /* Tell FMOD that we have loaded this FSB */
                    ERRCHECK(result = FMOD_EventSystem_PreloadFSB(eventsystem, "tutorial_bank.fsb", 0, samplebank));

                    /* Now we can get the event without diskaccess */
                    ERRCHECK(result = FMOD_EventSystem_GetEvent(eventsystem, "examples/FeatureDemonstration/Basics/SimpleEvent", FMOD_EVENT_ERROR_ON_DISKACCESS, &sampleevent));
                }
                printf("Sample event ready\n");
            }
            else if (key == 'E')
            {
                if (!samplebank)
                {
                    printf("Nothing to unload\n");
                }
                else
                {
                    ERRCHECK(result = FMOD_Event_Stop(sampleevent, 0));
                    sampleevent = 0;
                    /* Note: we *MUST* call unload before freeing data */
                    ERRCHECK(result = FMOD_EventSystem_UnloadFSB(eventsystem, "tutorial_bank.fsb", 0));
                    ERRCHECK(result = FMOD_Sound_Release(samplebank));
                    samplebank = 0;
                    if (samplebank_mem)
                    {
                        free(samplebank_mem);
                        samplebank_mem = 0;
                    }
                    printf("Event data unloaded\n");
                }
            }
            else if (key == 'd')
            {
                if (!sampleevent)
                {
                    printf("no event loaded!\n");
                }
                else
                {
                    ERRCHECK(result = FMOD_Event_Start(sampleevent));
                }
            }
            else if (key == 'w')
            {
                /* Attempt to get the event without opening a new stream */
                result = FMOD_EventSystem_GetEvent(eventsystem, "examples/AdvancedTechniques/MultiChannelMusic", FMOD_EVENT_ERROR_ON_DISKACCESS, &streamevent);
                if (result != FMOD_ERR_FILE_UNWANTED)
                {
                    ERRCHECK(result);
                }
                else
                {
                    /*  file unwanted error tells us we haven't got the stream instance preloaded so preload it now */
                    printf("Opening stream\n");

                    /* If the 'Max Streams' property of the sound bank is greater than 1 then mutiple streams can be opened. This means we need
                       to preload it multiple times if we want to prevent FMOD from creating streams internally. Each stream is uniquely identified
                       using the 'streaminstance' parameter to preloadFSB which counts upwards from 0.
                    */
                    ERRCHECK(result = FMOD_System_CreateSound(system, "..\\media\\streaming_bank.fsb", FMOD_CREATESTREAM, 0, &streambank));
                    ERRCHECK(result = FMOD_EventSystem_PreloadFSB(eventsystem, "streaming_bank.fsb", 0, streambank));
                    
                    /* Now we can get the event without opening a new stream */
                    ERRCHECK(result = FMOD_EventSystem_GetEvent(eventsystem, "examples/AdvancedTechniques/MultiChannelMusic", FMOD_EVENT_ERROR_ON_DISKACCESS, &streamevent));
                }
                printf("Stream event ready\n");
            }
            else if (key == 'W')
            {
                if (!streambank)
                {
                    printf("Nothing to unload\n");
                }
                else
                {
                    ERRCHECK(result = FMOD_Event_Stop(streamevent, 0));
                    streamevent = 0;
                    /* Note: we *MUST* call unload before releasing stream */
                    ERRCHECK(result = FMOD_EventSystem_UnloadFSB(eventsystem, "streaming_bank.fsb", 0));
                    ERRCHECK(result = FMOD_Sound_Release(streambank));
                    streambank = 0;
                    printf("Stream closed\n");
                }
            }
            else if (key == 's')
            {
                if (!streamevent)
                {
                    printf("no event loaded!\n");
                }
                else
                {
                    ERRCHECK(result = FMOD_Event_Start(streamevent));
                }
            }
        }

        ERRCHECK(result = FMOD_EventSystem_Update(eventsystem));
        Sleep(15);

    } while (key != 27);
    
    if (samplebank)
    {
        /* Note: we *MUST* call unload before freeing data */
        ERRCHECK(result = FMOD_EventSystem_UnloadFSB(eventsystem, "tutorial_bank.fsb", 0));
        ERRCHECK(result = FMOD_Sound_Release(samplebank));
        if (samplebank_mem)
        {
            free(samplebank_mem);
        }
    }
    
    if (streambank)
    {
        /* Note: we *MUST* call unload before releasing stream */
        ERRCHECK(result = FMOD_EventSystem_UnloadFSB(eventsystem, "streaming_bank.fsb", 0));
        ERRCHECK(result = FMOD_Sound_Release(streambank));
    }

    ERRCHECK(result = FMOD_EventSystem_Release(eventsystem));

    /* Free the memory we have allocated */
    free(project_mem);

    return 0;
}
