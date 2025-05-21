/*===============================================================================================
 Stream IO Example
 Copyright (c), Firelight Technologies Pty, Ltd 2004-2016.

 This example shows how to play a stream and use a custom file handler that defers reads for the
 streaming part.  FMOD will allow the user to return straight away from a file read request and
 supply the data at a later time.
===============================================================================================*/
#include <windows.h>
#include <process.h> 
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

CRITICAL_SECTION gCrit;

typedef struct dataqueue
{
    struct dataqueue *next;
    FMOD_ASYNCREADINFO *info;
} dataqueue;

dataqueue queuehead = { &queuehead, 0 };
bool gThreadQuit = false;
bool gSleepBreak = false;


/*
    File callbacks
*/
FMOD_RESULT F_CALLBACK myopen(const char *name, int unicode, unsigned int *filesize, void **handle, void **userdata)
{
    if (name)
    {
        FILE *fp;

        fp = fopen(name, "rb");
        if (!fp)
        {
            return FMOD_ERR_FILE_NOTFOUND;
        }

        fseek(fp, 0, SEEK_END);
        *filesize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        *userdata = (void *)0x12345678;
        *handle = fp;
    }

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK myclose(void *handle, void *userdata)
{
    if (!handle)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    fclose((FILE *)handle);

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK myread(void *handle, void *buffer, unsigned int sizebytes, unsigned int *bytesread, void *userdata)
{
    if (!handle)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    if (bytesread)
    {
        *bytesread = (int)fread(buffer, 1, sizebytes, (FILE *)handle);   
        if (*bytesread < sizebytes)
        {
            return FMOD_ERR_FILE_EOF;
        }
    }


    return FMOD_OK;
}


FMOD_RESULT F_CALLBACK myseek(void *handle, unsigned int pos, void *userdata)
{
    printf("************ seek to %d\n", pos);
    if (!handle)
    {
        return FMOD_ERR_INVALID_PARAM;
    }
   
    fseek((FILE *)handle, pos, SEEK_SET);

    return FMOD_OK;
}


FMOD_RESULT F_CALLBACK myasyncread(FMOD_ASYNCREADINFO *info, void *userdata)
{
    dataqueue *current;

    printf("REQUESTING %5d bytes from offset %5d PRIORITY = %d.\n", info->sizebytes, info->offset, info->priority);
    
    current = (dataqueue *)malloc(sizeof(dataqueue));
    if (!current)
    {
        info->result = FMOD_ERR_MEMORY;
        return FMOD_ERR_MEMORY;
    }
    
    EnterCriticalSection(&gCrit);

    current->next = queuehead.next;    
    current->info = info;
    queuehead.next = current;
    
    if (info->priority > 50)
    {
        gSleepBreak = true;     /* Tell file thread to stop sleeping, we have an urgent request here! */
    }
    
    LeaveCriticalSection(&gCrit);

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK myasynccancel(void *handle, void *userdata)
{
    dataqueue *current;
    
    EnterCriticalSection(&gCrit);
    current = queuehead.next;

    while (current != &queuehead)
    {    
        if (current->info->handle == handle)
        {
            queuehead.next = current->next;
            free(current);
            current = queuehead.next;
        }
        else
        {
            current = current->next;
        }
    }

    LeaveCriticalSection(&gCrit);

    return FMOD_OK;
}


unsigned int __stdcall ProcessQueue(void *param)
{
    while (!gThreadQuit)
    {
        int count;
        dataqueue *current;
        
        EnterCriticalSection(&gCrit);

        current = queuehead.next;
        while (current != &queuehead)
        {    
            unsigned int toread = current->info->sizebytes;
            if (toread > 16384)
            {
                toread = 16384;     /* For fun - Let's deprive the read of the whole block.  Only give 16kb at a time to make it re-ask for more later. */
            }           

            for (count = 0; count < 50; count++)
            {
                Sleep(10);
                if (gSleepBreak)
                {
                    printf("URGENT REQUEST - reading now!\n");
                    gSleepBreak = false;
                    break;
                }
            }
            
            fseek((FILE *)current->info->handle, current->info->offset, SEEK_SET);           
            current->info->bytesread = fread(current->info->buffer, 1, toread, (FILE *)current->info->handle);
            
            if (current->info->bytesread < toread)
            {
                printf("FED        %5d bytes from offset %5d (* EOF)\n", current->info->bytesread, current->info->offset);
                current->info->result = FMOD_ERR_FILE_EOF;
            }
            else
            {
                printf("FED        %5d bytes from offset %5d\n", current->info->bytesread, current->info->offset);
                current->info->result = FMOD_OK;
            }
                       
            queuehead.next = current->next;
            free(current);
            current = queuehead.next;
        }

        LeaveCriticalSection(&gCrit);       
    }
    
    _endthreadex(0);
    
    return 0;
}



int main(int argc, char *argv[])
{
    FMOD::System     *system;
    FMOD::Sound      *sound;
    FMOD::Channel    *channel = 0;
    FMOD_RESULT       result;
    int               key;
    unsigned int      version;
    HANDLE            threadhandle;

    InitializeCriticalSection(&gCrit);
    
	threadhandle = (HANDLE)_beginthreadex(NULL, 0, ProcessQueue, 0, 0, 0);
    if (!threadhandle)
    {
        printf("Failed to create file thread.\n");
        return 0;
    }
    
    /*
        Create a System object and initialize.
    */
    result = FMOD::System_Create(&system);
    ERRCHECK(result);

    result = system->getVersion(&version);
    ERRCHECK(result);

    if (version < FMOD_VERSION)
    {
        printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
        return 0;
    }

    result = system->init(1, FMOD_INIT_NORMAL, 0);
    ERRCHECK(result);

    result = system->setStreamBufferSize(32768, FMOD_TIMEUNIT_RAWBYTES);
    ERRCHECK(result);
    
    result = system->setFileSystem(myopen, myclose, myread, myseek, myasyncread, myasynccancel, 2048);
    ERRCHECK(result);

    printf("====================================================================\n");
    printf("Stream IO Example.  Copyright (c) Firelight Technologies 2004-2016.\n");
    printf("====================================================================\n");
    printf("\n");
    printf("\n");
    printf("====================== CALLING CREATESOUND ON MP3 =======================\n");
    
    result = system->createStream("../media/wave.mp3", FMOD_SOFTWARE | FMOD_LOOP_NORMAL | FMOD_2D | FMOD_IGNORETAGS, 0, &sound);
    ERRCHECK(result);

    printf("====================== CALLING PLAYSOUND ON MP3 =======================\n");

    result = system->playSound(FMOD_CHANNEL_FREE, sound, false, &channel);
    ERRCHECK(result);

    /*
        Main loop.
    */
    do
    {
        if (sound)
        {
            FMOD_OPENSTATE openstate;
            bool starving;

            sound->getOpenState(&openstate, 0, &starving, 0);
            
            if (starving)
            {
                printf("Starving\n");
                result = channel->setMute(true);
            }
            else
            {
                result = channel->setMute(false);
                ERRCHECK(result);
            }
        }

       
        if (_kbhit())
        {
            key = _getch();

            switch (key)
            {
                case ' ' :
                {
                    result = sound->release();
                    if (result == FMOD_OK)
                    {
                        sound = 0;
                        printf("Released sound.\n");
                    }
                    break;
                }
            }
        }

        system->update();
        Sleep(10);

    } while (key != 27);

    printf("\n");

    /*
        Shut down
    */
    if (sound)
    {
        result = sound->release();
        ERRCHECK(result);
    }
    result = system->close();
    ERRCHECK(result);
    result = system->release();
    ERRCHECK(result);

    gThreadQuit = true;   

    return 0;
}
