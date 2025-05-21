/*
    This example builds the FSB's required for the Designer API examples.
    NOTE: the order of files here represents the output order of Designer.
*/

#include "fsbank.h"
#include <stdio.h>
#include <malloc.h>
#include <conio.h>
#include <string.h>

const char *gTutorialFileNames[] =
{
    "../../../fmoddesignerapi/examples/media/music/flsh-idle-05.ogg",
    "../../../fmoddesignerapi/examples/media/cycle-stop.ogg",
    "../../../fmoddesignerapi/examples/media/music/layer-guitar-d.ogg",
    "../../../fmoddesignerapi/examples/media/note.ogg",
    "../../../fmoddesignerapi/examples/media/ambient_two.ogg",
    "../../../fmoddesignerapi/examples/media/music/state-race-finish.ogg",
    "../../../fmoddesignerapi/examples/media/music/layer-guitar-a.ogg",
    "../../../fmoddesignerapi/examples/media/max_play_behavior_two.ogg",
    "../../../fmoddesignerapi/examples/media/music/state-idle-fadeout.ogg",
    "../../../fmoddesignerapi/examples/media/car/onmid.ogg",
    "../../../fmoddesignerapi/examples/media/sequence-three.ogg",
    "../../../fmoddesignerapi/examples/media/car/offhigh.ogg",
    "../../../fmoddesignerapi/examples/media/music/layer-drums-d.ogg",
    "../../../fmoddesignerapi/examples/media/car/idle.ogg",
    "../../../fmoddesignerapi/examples/media/car/offlow.ogg",
    "../../../fmoddesignerapi/examples/media/music/layer-guitar-c.ogg",
    "../../../fmoddesignerapi/examples/media/music/state-race-02a.ogg",
    "../../../fmoddesignerapi/examples/media/music/flsh-idle-02.ogg",
    "../../../fmoddesignerapi/examples/media/drum-loop.ogg",
    "../../../fmoddesignerapi/examples/media/music/state-idle-01.ogg",
    "../../../fmoddesignerapi/examples/media/sequence-start.ogg",
    "../../../fmoddesignerapi/examples/media/music/state-race-01a.ogg",
    "../../../fmoddesignerapi/examples/media/ambient_one_four.ogg",
    "../../../fmoddesignerapi/examples/media/music/flsh-idle-01.ogg",
    "../../../fmoddesignerapi/examples/media/max_play_behavior_one.ogg",
    "../../../fmoddesignerapi/examples/media/cycle-start.ogg",
    "../../../fmoddesignerapi/examples/media/music/layer-guitar-alt-c.ogg",
    "../../../fmoddesignerapi/examples/media/music/state-idle-02.ogg",
    "../../../fmoddesignerapi/examples/media/ambient_three.ogg",
    "../../../fmoddesignerapi/examples/media/car/offmid.ogg",
    "../../../fmoddesignerapi/examples/media/music/excited.ogg",
    "../../../fmoddesignerapi/examples/media/music/flsh-idle-04.ogg",
    "../../../fmoddesignerapi/examples/media/car/onlow.ogg",
    "../../../fmoddesignerapi/examples/media/music/state-idle-03.ogg",
    "../../../fmoddesignerapi/examples/media/sequence-four.ogg",
    "../../../fmoddesignerapi/examples/media/music/state-race-02b.ogg",
    "../../../fmoddesignerapi/examples/media/sequence-one.ogg",
    "../../../fmoddesignerapi/examples/media/music/layer-drums-a.ogg",
    "../../../fmoddesignerapi/examples/media/music/layer-guitar-alt-d.ogg",
    "../../../fmoddesignerapi/examples/media/music/flsh-idle-03.ogg",
    "../../../fmoddesignerapi/examples/media/ambient_one_loop.ogg",
    "../../../fmoddesignerapi/examples/media/music/layer-bass-a.ogg",
    "../../../fmoddesignerapi/examples/media/interactive_music_2.ogg",
    "../../../fmoddesignerapi/examples/media/music/layer-guitar-b.ogg",
    "../../../fmoddesignerapi/examples/media/music/layer-guitar-alt-a.ogg",
    "../../../fmoddesignerapi/examples/media/max_play_behavior_four.ogg",
    "../../../fmoddesignerapi/examples/media/cycle-sustain.ogg",
    "../../../fmoddesignerapi/examples/media/music/state-race-start.ogg",
    "../../../fmoddesignerapi/examples/media/music/relaxed.ogg",
    "../../../fmoddesignerapi/examples/media/music/state-race-01b.ogg",
    "../../../fmoddesignerapi/examples/media/car/onhigh.ogg",
    "../../../fmoddesignerapi/examples/media/sequence-two.ogg",
    "../../../fmoddesignerapi/examples/media/music/layer-drums-b.ogg",
    "../../../fmoddesignerapi/examples/media/interactive_music_3.ogg",
    "../../../fmoddesignerapi/examples/media/sequence-end.ogg",
    "../../../fmoddesignerapi/examples/media/max_play_behavior_three.ogg",
    "../../../fmoddesignerapi/examples/media/interactive_music_1.ogg",
    "../../../fmoddesignerapi/examples/media/music/layer-drums-c.ogg",
    "../../../fmoddesignerapi/examples/media/music/layer-guitar-alt-b.ogg"
};

const char *gStreamingFileNames[] =
{
    "../../../fmoddesignerapi/examples/media/interactive_music_6channel.ogg"
};

const unsigned int gNumTutorialFileNames  = sizeof(gTutorialFileNames) / sizeof(char *);
const unsigned int gNumStreamingFileNames = sizeof(gStreamingFileNames) / sizeof(char *);

#define CHECK_RESULT(_x) { FSBANK_RESULT _result = (_x); if (_result != FSBANK_OK) { printf("\nBuild failed, Press any key to exit.\n"); _getch(); return _result; } } 

FSBANK_RESULT printErrors(const FSBANK_SUBSOUND *subSounds)
{
    FSBANK_RESULT result                    = FSBANK_OK;
    const         FSBANK_PROGRESSITEM *item = NULL;

    printf("Build Error!\n");

    do
    {
        result = FSBank_FetchNextProgressItem(&item);
        CHECK_RESULT(result);

        if (item)
        {
            if (item->state == FSBANK_STATE_FAILED)
            {
                const FSBANK_STATEDATA_FAILED *failedStateData = (const FSBANK_STATEDATA_FAILED *)item->stateData;
                printf("Error: %s -- \"%s\"\n", failedStateData->errorString, item->subSoundIndex < 0 ? "System" : subSounds[item->subSoundIndex].fileNames[0]);
            }

            result = FSBank_ReleaseProgressItem(item);
            CHECK_RESULT(result);
        }
    } while (item != NULL);

    return FSBANK_OK;
}


FSBANK_RESULT buildBank(const char **fileNames, unsigned int numFileNames, const char *outputFileName)
{
    FSBANK_RESULT    result    = FSBANK_OK;
    FSBANK_SUBSOUND *subSounds = (FSBANK_SUBSOUND *)alloca(numFileNames * sizeof(FSBANK_SUBSOUND));

    for (unsigned int i = 0; i < numFileNames; i++)
    {
        memset(&subSounds[i], 0, sizeof(FSBANK_SUBSOUND));

        subSounds[i].fileNames          = &fileNames[i];
        subSounds[i].numFileNames       = 1;
        subSounds[i].speakerMap         = FSBANK_SPEAKERMAP_DEFAULT;        
        subSounds[i].overrideFlags      = FSBANK_BUILD_DEFAULT;
        subSounds[i].overrideQuality    = 0;
        subSounds[i].desiredSampleRate  = 0;
    }

    printf("Building %s...\n", outputFileName);

    result = FSBank_Build(subSounds, numFileNames, FSBANK_FORMAT_MP3, FSBANK_BUILD_DEFAULT | FSBANK_BUILD_DONTLOOP, 0, NULL, outputFileName);
    if (result != FSBANK_OK)
    {
        printErrors(subSounds);
        return result;
    }

    printf("done.\n");
    return FSBANK_OK;
}


int main(void)
{
    FSBANK_RESULT result = FSBANK_OK;

    result = FSBank_Init(FSBANK_FSBVERSION_FSB4, FSBANK_INIT_NORMAL | FSBANK_INIT_GENERATEPROGRESSITEMS, 2, NULL);
    CHECK_RESULT(result);

    result = buildBank(gTutorialFileNames, gNumTutorialFileNames, "tutorial_bank.fsb");
    CHECK_RESULT(result);

    result = buildBank(gStreamingFileNames, gNumStreamingFileNames, "streaming_bank.fsb");
    CHECK_RESULT(result);

    result = FSBank_Release();
    CHECK_RESULT(result);

    printf("\nBuild complete, Press any key to exit.\n");
    _getch();

    return FSBANK_OK;
}

