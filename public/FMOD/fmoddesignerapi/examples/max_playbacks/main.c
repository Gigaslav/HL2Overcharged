/*=============================================================================
 Max Playbacks Example
 Copyright (c), Firelight Technologies Pty, Ltd 2004-2016.

 Demonstrates basic usage of event max playbacks behaviour.
=============================================================================*/
#include "../../api/inc/fmod_event.h"
#include "../../api/inc/fmod_errors.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

const char   *MEDIA_PATH = "..\\media\\";

static float g_distance = 2.0f;
static float g_sound    = 0.0f;


void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }
}


void setupEvent(FMOD_EVENT *event)
{
    FMOD_RESULT           result;
    FMOD_EVENTPARAMETER  *param;
    FMOD_VECTOR           pos = {0, 0, g_distance};

    ERRCHECK(result = FMOD_Event_Set3DAttributes(event, &pos, 0, 0));
    ERRCHECK(result = FMOD_Event_GetParameter(event, "sound", &param));
    ERRCHECK(result = FMOD_EventParameter_SetValue(param, g_sound));
}


void stopAllEvents(FMOD_EVENTGROUP *eventgroup)
{
    ERRCHECK(FMOD_EventGroup_FreeEventData(eventgroup, 0, 1));
}


int main(int argc, char *argv[])
{
    FMOD_RESULT        result;
    FMOD_EVENTSYSTEM  *eventsystem;
    FMOD_EVENTGROUP   *eventgroup;
    FMOD_EVENT        *event;
    int                key;

    enum
    {
        StealOldest,
        StealNewest,
        StealQuietest,
        JustFail,
        JustFailIfQuietest
    } event_behaviour = StealOldest;

    ERRCHECK(result = FMOD_EventSystem_Create(&eventsystem));
    ERRCHECK(result = FMOD_EventSystem_Init(eventsystem, 64, FMOD_INIT_NORMAL, 0, FMOD_EVENT_INIT_NORMAL));
    ERRCHECK(result = FMOD_EventSystem_SetMediaPath(eventsystem, (char *)MEDIA_PATH));
    ERRCHECK(result = FMOD_EventSystem_Load(eventsystem, "examples.fev", 0, 0));
    ERRCHECK(result = FMOD_EventSystem_GetGroup(eventsystem, "examples/FeatureDemonstration/MaxPlayback", FMOD_EVENT_DEFAULT, &eventgroup));
    ERRCHECK(result);

    printf("======================================================================\n");
    printf("Max Playbacks Example.  Copyright (c) Firelight Technologies 2004-2016.\n");
    printf("----------------------------------------------------------------------\n");
    printf("Press '1'   to select 'Steal oldest' behaviour\n");
    printf("Press '2'   to select 'Steal newest' behaviour\n");
    printf("Press '3'   to select 'Steal quietest' behaviour\n");
    printf("Press '4'   to select 'Just fail' behaviour\n");
    printf("Press '5'   to select 'Just fail if quietest' behaviour\n");
    printf("Press Space to start an event\n");
    printf("Press 's'   to stop all events\n");
    printf("Press '>'   to increase event distance\n");
    printf("Press '<'   to decrease event distance\n");
    printf("Press ESC to quit\n");
    printf("======================================================================\n");

    key = 0;
    do
    {
        if (_kbhit())
        {
            char *name;
            key = _getch();

            switch(key)
            {
                case '1' : // 'Steal oldest'
                    event_behaviour = StealOldest;
                    stopAllEvents(eventgroup);
                    break;

                case '2' : // 'Steal newest'
                    event_behaviour = StealNewest;
                    stopAllEvents(eventgroup);
                    break;

                case '3' : // 'Steal quietest'
                    event_behaviour = StealQuietest;
                    stopAllEvents(eventgroup);
                    break;

                case '4' : // 'Just fail'
                    event_behaviour = JustFail;
                    stopAllEvents(eventgroup);
                    break;

                case '5' : // 'Just fail if quietest'
                    event_behaviour = JustFailIfQuietest;
                    stopAllEvents(eventgroup);
                    break;

                case ' ' : // Play an event

                    switch(event_behaviour)
                    {
                        case StealOldest :
                            name = "MaxPlay-StealOldest";
                            break;

                        case StealNewest :
                            name = "MaxPlay-StealNewest";
                            break;

                        case StealQuietest :
                            name = "MaxPlay-StealQuietest";
                            break;

                        case JustFail :
                            name = "MaxPlay-JustFail";
                            break;

                        case JustFailIfQuietest :
                            name = "MaxPlay-JustFailIfQuietest";
                            break;
                    }

                    // Clear the line
                    printf("%79s\r", " ");

                    if (event_behaviour == JustFailIfQuietest)
                    {
                        /* The 'Just fail if quietest' behaviour calculates the expected
                           volume of the event based on the properties of the info-only
                           event, so we have to get the info-only event first and set it
                           up appropriately.
                         */

                        // get the info-only event to set up for volume calculation
                        ERRCHECK(result = FMOD_EventGroup_GetEvent(eventgroup, name, FMOD_EVENT_INFOONLY, &event));

                        /* set the desired properties on the info-only event
                           Notes:
                           - distances below the event's 3D Min Distance all give the
                             same volume; in this case, getEvent will just fail
                           - we could set other volume-affecting properties here as
                             well (e.g. orientation if the event has a falloff cone)
                         */
                        setupEvent(event);

                        // attempt to get a real event instance
                        result = FMOD_EventGroup_GetEvent(eventgroup, name, FMOD_EVENT_DEFAULT, &event);
                        if (result == FMOD_OK)
                        {
                            FMOD_EVENTPARAMETER *param;

                            printf("getEvent(\"%s\") succeeded\n", name);

                            /* we don't need to set the position of the instance,
                               as it is copied from the info-only event, but we
                               do need to set the parameter value.
                             */
                            ERRCHECK(result = FMOD_Event_GetParameter(event, "sound", &param));
                            ERRCHECK(result = FMOD_EventParameter_SetValue(param, g_sound));
                            ERRCHECK(result = FMOD_Event_Start(event));
                        }
                        else
                        {
                            printf("getEvent(\"%s\") failed\n", name);
                        }
                    }
                    else
                    {
                        result = FMOD_EventGroup_GetEvent(eventgroup, name, FMOD_EVENT_DEFAULT, &event);
                        if (result == FMOD_OK)
                        {
                            printf("getEvent(\"%s\") succeeded\n", name);
                            setupEvent(event);
                            ERRCHECK(result = FMOD_Event_Start(event));
                        }
                        else
                        {
                            printf("getEvent(\"%s\") failed\n", name);
                        }
                    }

                    ++g_sound;
                    if(g_sound > 3)
                    {
                        g_sound = 0;
                    }

                    break;

                case 's' :
                    stopAllEvents(eventgroup);
                    break;

                case '>' : case '.' :
                    g_distance += 0.1f;
                    break;

                case '<' : case ',' :
                    g_distance -= 0.1f;
                    g_distance = (g_distance < 0.0f) ? 0.0f : g_distance;
                    break;
            }


            switch(event_behaviour)
            {
                case StealOldest:
                    name = "Steal oldest";
                    break;

                case StealNewest:
                    name = "Steal newest";
                    break;

                case StealQuietest:
                    name = "Steal quietest";
                    break;

                case JustFail:
                    name = "Just fail";
                    break;

                case JustFailIfQuietest:
                    name = "Just fail if quietest";
                    break;
            }

            printf("Sound = %1.0f, Distance = %4.1f, Behaviour = %-25s\r", g_sound, g_distance, name);
        }

        ERRCHECK(result = FMOD_EventSystem_Update(eventsystem));
        Sleep(10);

    } while (key != 27);

    ERRCHECK(result = FMOD_EventSystem_Unload(eventsystem));
    ERRCHECK(result = FMOD_EventSystem_Release(eventsystem));
    return 0;
}
