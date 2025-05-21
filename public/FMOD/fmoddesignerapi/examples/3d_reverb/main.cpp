/*==================================================================================================
 3D Reverb Example
 Copyright (c), Firelight Technologies Pty, Ltd 2005-2016.

 Example to demonstrate 3d reverb spheres, global reverb, multiple reverb instances and music reverb
===================================================================================================*/

#include "../../api/inc/fmod_event.hpp"
#include "../../api/inc/fmod_errors.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>

#include "examples.h"

const int                           INTERFACE_UPDATETIME = 80;                          // 80ms update for interface
const float                         DISTANCEFACTOR = 1.0f;                              // Units per meter.  I.e feet would = 3.28
FMOD_VECTOR                         listenerpos  = { 0.0f, 0.0f, 0.0f * DISTANCEFACTOR };
bool                                listenerflag = true;


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
    FMOD::EventSystem               *eventsystem;
    FMOD::Event                     *event;
    FMOD::MusicSystem               *musicsystem;
    FMOD::MusicPrompt               *musicprompt;
    FMOD::EventReverb               *eventreverb1, *eventreverb2;

    FMOD_REVERB_CHANNELPROPERTIES    reverbchannelproperties;    
    FMOD_VECTOR                      reverbposition;
    float                            mindistance, maxdistance;
    FMOD_RESULT                      result;

    printf("======================================================================\n");
    printf("3D Reverb Example.  Copyright (c) Firelight Technologies 2004-2016.\n");
    printf("======================================================================\n");
    printf("This demonstrates 3d reverb spheres as well as global reverb,\n");
    printf("multiple reverb instances and music reverb\n");
    printf("======================================================================\n\n");

    /*
        Create a event system object and initialize.
    */
    ERRCHECK(result = FMOD::EventSystem_Create(&eventsystem));
    ERRCHECK(result = eventsystem->init(64, FMOD_INIT_3D_RIGHTHANDED, 0, FMOD_EVENT_INIT_NORMAL));
    ERRCHECK(result = eventsystem->setMediaPath("..\\media\\"));
    ERRCHECK(result = eventsystem->load("examples.fev", 0, 0));
    ERRCHECK(result = eventsystem->getEvent("examples/FeatureDemonstration/Basics/FootStep", FMOD_EVENT_DEFAULT, &event));
    ERRCHECK(result = eventsystem->getMusicSystem(&musicsystem));
    ERRCHECK(result = musicsystem->prepareCue(MUSICCUE_EXAMPLES_ENTER_MENU__VIBES_, &musicprompt));
    ERRCHECK(result = eventsystem->createReverb(&eventreverb1));
    ERRCHECK(result = eventsystem->createReverb(&eventreverb2));

    /*
        Initialize 3D reverb objects
        (uses global reverb instance #0)
    */    
    FMOD_REVERB_PROPERTIES sphere1reverb = FMOD_PRESET_HANGAR;
    reverbposition.x = -10.0;
    reverbposition.y = 0;
    reverbposition.z = 0;
    mindistance = 10.0;
    maxdistance = 20.0;
    ERRCHECK(result = eventreverb1->setProperties(&sphere1reverb));
    ERRCHECK(result = eventreverb1->set3DAttributes(&reverbposition, mindistance, maxdistance));
    ERRCHECK(result = eventreverb1->setActive(true));

    FMOD_REVERB_PROPERTIES sphere2reverb = FMOD_PRESET_SEWERPIPE;
    reverbposition.x = 10.0;
    reverbposition.y = 0;
    reverbposition.z = 0;
    mindistance = 10.0;
    maxdistance = 20.0;
    ERRCHECK(result = eventreverb2->setProperties(&sphere2reverb));
    ERRCHECK(result = eventreverb2->set3DAttributes(&reverbposition, mindistance, maxdistance));
    ERRCHECK(result = eventreverb2->setActive(true));

    /*
        Setup ambient reverb
    */
    FMOD_REVERB_PROPERTIES ambientreverb = FMOD_PRESET_ROOM;
    ERRCHECK(result = eventsystem->setReverbAmbientProperties(&ambientreverb));

    /*
        Setup global reverb instance #1 for music
    */
    FMOD_REVERB_PROPERTIES musicreverb = FMOD_PRESET_GENERIC;
    musicreverb.Room = 0;
    musicreverb.Instance = 1;
    ERRCHECK(result = eventsystem->setReverbProperties(&musicreverb));

    /*
        Turn off global reverb instance #1 for event
    */
    memset(&reverbchannelproperties, 0, sizeof(FMOD_REVERB_CHANNELPROPERTIES));
    reverbchannelproperties.Flags = FMOD_REVERB_CHANNELFLAGS_INSTANCE1;
    ERRCHECK(result = event->getReverbProperties(&reverbchannelproperties));
    reverbchannelproperties.Room = -10000;                                          // wet level -10000 sets instance #1 off for this event
    ERRCHECK(result = event->setReverbProperties(&reverbchannelproperties));

    /*
        Start music and event
    */
    ERRCHECK(result = musicprompt->begin());
    ERRCHECK(result = musicsystem->setVolume(0.5f));

    FMOD_VECTOR eventposition = {0, 0, 5.0f};
    FMOD_VECTOR eventvelocity = {0, 0, 0};
    ERRCHECK(event->set3DAttributes(&eventposition, &eventvelocity));
    ERRCHECK(event->start());


    printf("======================================================================\n");
    printf("Press 'm'           to activate/deactivate music.\n");
    printf("Press '+' / '-'     to decrease/increase music reverb wet level.\n");
    printf("Press 'e'           to activate/deactivate event.\n");
    printf("Press ' '           to toggle auto listener update mode.\n");
    printf("Press '<' / '>'     to move listener while in auto update mode.\n");
    printf("Press ESC   to quit\n");
    printf("----------------------------------------------------------------------\n");
    printf("1, 2 : centre of sphere; m : minimum distance; x : maximum distance   \n");
    printf("======================================================================\n\n");

    int key = 0;
    FMOD_EVENT_STATE state;

    /*
        Main loop
    */
    {
        do
        {
            if (kbhit())
            {
                key = getch();

                // Toggle event on and off
                if (key == 'e' || key == 'E')
                {
                    ERRCHECK(result = event->getState(&state));
                    if (state & FMOD_EVENT_STATE_PLAYING)
                    {
                        ERRCHECK(result = event->stop());
                    }
                    else
                    {
                        ERRCHECK(result = event->start());
                    }
                }

                // Toggle music on and off
                if (key == 'm' || key == 'M')
                {
                    bool active;
                    ERRCHECK(result = musicprompt->isActive(&active));
                    if (active)
                    {
                        ERRCHECK(result = musicprompt->end());
                        ERRCHECK(result = musicsystem->reset());
                    }
                    else
                    {
                        ERRCHECK(result = musicprompt->begin());
                    }
                }

                // Increase music reverb
                if (key == '+' || key == '=')
                {
                    reverbchannelproperties.Flags = FMOD_REVERB_CHANNELFLAGS_INSTANCE1;
                    ERRCHECK(result = musicsystem->getReverbProperties(&reverbchannelproperties));

                    reverbchannelproperties.Room += 100;
                    if (reverbchannelproperties.Room > 0)
                    {
                        reverbchannelproperties.Room = 0;
                    }

                    ERRCHECK(result = musicsystem->setReverbProperties(&reverbchannelproperties));
                }

                // Decrease music reverb
                if (key == '-' || key == '_')
                {
                    reverbchannelproperties.Flags = FMOD_REVERB_CHANNELFLAGS_INSTANCE1;
                    ERRCHECK(result = musicsystem->getReverbProperties(&reverbchannelproperties));

                    reverbchannelproperties.Room -= 100;
                    if (reverbchannelproperties.Room < -2000)
                    {
                        reverbchannelproperties.Room = -2000;
                    }

                    ERRCHECK(result = musicsystem->setReverbProperties(&reverbchannelproperties));
                }

                // Toggle listener update
                if (key == ' ')
                {
                    listenerflag = !listenerflag;
                }

                if (!listenerflag)
                {
                    if (key == '<' || key == ',')
                    {
                        listenerpos.x -= 1.0f * DISTANCEFACTOR;
                        if (listenerpos.x < -35 * DISTANCEFACTOR)
                        {
                            listenerpos.x = -35 * DISTANCEFACTOR;
                        }
                    }
                    if (key == '>' || key == '.') 
                    {
                        listenerpos.x += 1.0f * DISTANCEFACTOR;
                        if (listenerpos.x > 36 * DISTANCEFACTOR)
                        {
                            listenerpos.x = 36 * DISTANCEFACTOR;
                        }
                    }
                }
            }

            // ==========================================================================================
            // UPDATE THE LISTENER
            // ==========================================================================================
            {
                static float t = 0;
                static FMOD_VECTOR lastpos = { 0.0f, 0.0f, 0.0f };
                FMOD_VECTOR forward        = { 0.0f, 0.0f, 1.0f };
                FMOD_VECTOR up             = { 0.0f, 1.0f, 0.0f };
                FMOD_VECTOR vel;

                if (listenerflag)
                {
                    listenerpos.x = (float)sin(t * 0.05f) * 33.0f * DISTANCEFACTOR; // left right pingpong
                }

                // vel = how far we moved last FRAME (m/f), then time compensate it to SECONDS (m/s).
                vel.x = (listenerpos.x - lastpos.x) * (1000 / INTERFACE_UPDATETIME);
                vel.y = (listenerpos.y - lastpos.y) * (1000 / INTERFACE_UPDATETIME);
                vel.z = (listenerpos.z - lastpos.z) * (1000 / INTERFACE_UPDATETIME);

                // store pos for next time
                lastpos = listenerpos;

                result = eventsystem->set3DListenerAttributes(0, &listenerpos, &vel, &forward, &up);
                ERRCHECK(result);

                t += (30 * (1.0f / (float)INTERFACE_UPDATETIME));    // t is just a time value .. it increments in 30m/s steps in this example

                // print out a small visual display
                {
                    char s[80];

                    sprintf(s, ".....x.........m.........1.........|.........2.........m.........x.......");

                    s[(int)(listenerpos.x / DISTANCEFACTOR) + 35] = 'L';
                    printf("%s\r", s);
                }
            }

            eventsystem->update();

            Sleep(INTERFACE_UPDATETIME - 1);

        } while (key != 27);
    }

    ERRCHECK(result = eventsystem->release());
    return 0;
}
