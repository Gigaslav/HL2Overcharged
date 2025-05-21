#include "cbase.h"
#include "fmod_manager.h"
#if 0
using namespace FMOD;

System			*pSystem;
Sound			*pSound;
SoundGroup		*pSoundGroup;
Channel			*pChannel;
ChannelGroup	*pChannelGroup;
FMOD_RESULT		result;

CFMODManager gFMODMng;
CFMODManager* FMODManager()
{
	return &gFMODMng;
}

CFMODManager::CFMODManager()
{
	m_fFadeDelay = 0.0;
	newSoundFileToTransitionTo = "NULL";
	currentSound = "NULL";
	m_bShouldTransition = false;
	m_bFadeIn = false;
	m_bFadeOut = false;
}

CFMODManager::~CFMODManager()
{
	m_fFadeDelay = 0.0;
	newSoundFileToTransitionTo = "NULL";
	currentSound = "NULL";
	m_bShouldTransition = false;
	m_bFadeIn = false;
	m_bFadeOut = false;
}

// Starts FMOD
void CFMODManager::InitFMOD(void)
{
	result = System_Create(&pSystem); // Create the main system object.

	if (result != FMOD_OK)
		Warning("FMOD ERROR: System creation failed!\n");
	else
		DevMsg("FMOD system successfully created.\n");

	result = pSystem->init(100, FMOD_INIT_NORMAL, 0);   // Initialize FMOD system.
	/*char cDest[512] = { 0 };
	Q_snprintf(cDest, 512, "%s/sound/", engine->GetGameDirectory());
	result = pSystem->setOutput(FMOD_OUTPUTTYPE_WAVWRITER_NRT);
	result = pSystem->init(100, FMOD_INIT_NORMAL | FMOD_INIT_ENABLE_PROFILE, cDest);*/
	if (result != FMOD_OK)
		Warning("FMOD ERROR: Failed to initialize properly!\n");
	else
		DevMsg("FMOD initialized successfully.\n");
}

// Stops FMOD
void CFMODManager::ExitFMOD(void)
{
	result = pSystem->release();

	if (result != FMOD_OK)
		Warning("FMOD ERROR: System did not terminate properly!\n");
	else
		DevMsg("FMOD system terminated successfully.\n");
}

// Returns the full path of a specified sound file in the /sounds folder
const char* CFMODManager::GetFullPathToSound(const char* pathToFileFromModFolder)
{
	char* resultpath = new char[512];

	Q_snprintf(resultpath, 512, "%s/sound/%s", engine->GetGameDirectory(), pathToFileFromModFolder);

	// convert backwards slashes to forward slashes
	for (int i = 0; i < 512; i++)
	{
		if (resultpath[i] == '\\')
			resultpath[i] = '/';
	}

	return resultpath;
}

// Returns the name of the current ambient sound being played
// If there is an error getting the name of the ambient sound or if no ambient sound is currently being played, returns "NULL"
const char* CFMODManager::GetCurrentSoundName(void)
{
	return currentSound;
}

// Handles all fade-related sound stuffs
// Called every frame when the client is in-game
void CFMODManager::FadeThink(void)
{
	if (m_bFadeOut)
	{
		if (gpGlobals->curtime >= m_fFadeDelay)
		{
			float tempvol;
			pChannel->getVolume(&tempvol);

			if (tempvol > 0.0)
			{
				pChannel->setVolume(tempvol - 0.05);
				m_fFadeDelay = gpGlobals->curtime + 0.1;
			}
			else
			{
				pChannel->setVolume(0.0);
				m_bFadeOut = false;
				m_fFadeDelay = 0.0;
			}
		}
	}
	else if (m_bShouldTransition)
	{
		result = pSystem->createStream(GetFullPathToSound(newSoundFileToTransitionTo), FMOD_DEFAULT, 0, &pSound);

		if (result != FMOD_OK)
		{
			Warning("FMOD: Failed to create stream of sound '%s' ! (ERROR NUMBER: %i)\n", newSoundFileToTransitionTo, result);
			newSoundFileToTransitionTo = "NULL";
			m_bShouldTransition = false;
			return;
		}

		result = pSystem->playSound(FMOD_CHANNEL_REUSE, pSound, false, &pChannel);

		if (result != FMOD_OK)
		{
			Warning("FMOD: Failed to play sound '%s' ! (ERROR NUMBER: %i)\n", newSoundFileToTransitionTo, result);
			newSoundFileToTransitionTo = "NULL";
			m_bShouldTransition = false;
			return;
		}

		currentSound = newSoundFileToTransitionTo;
		newSoundFileToTransitionTo = "NULL";
		m_bShouldTransition = false;
	}
	else if (m_bFadeIn)
	{
		if (gpGlobals->curtime >= m_fFadeDelay)
		{
			float tempvol;
			pChannel->getVolume(&tempvol);

			if (tempvol < 1.0)
			{
				pChannel->setVolume(tempvol + 0.05);
				m_fFadeDelay = gpGlobals->curtime + 0.1;
			}
			else
			{
				pChannel->setVolume(1.0);
				m_bFadeIn = false;
				m_fFadeDelay = 0.0;
			}
		}
	}
}

// Compares specified ambient sound with the current ambient sound being played
// Returns true if they match, false if they do not or if no sound is being played
bool CFMODManager::IsSoundPlaying(const char* pathToFileFromSoundsFolder)
{
	const char* currentSoundPlaying = GetCurrentSoundName();

	return strcmp(currentSoundPlaying, pathToFileFromSoundsFolder) == 0;
}

// Abruptly starts playing a specified ambient sound
// In most cases, we'll want to use TransitionAmbientSounds instead
void CFMODManager::PlayAmbientSound(const char* pathToFileFromSoundsFolder, bool fadeIn)
{
	char cDest[512] = { 0 };
	Q_snprintf(cDest, 512, "%s/sound/%s", engine->GetGameDirectory(), pathToFileFromSoundsFolder);
	// convert backwards slashes to forward slashes
	for (int i = 0; i < 512; i++)
	{
		if (cDest[i] == '\\')
			cDest[i] = '/';
	}
	result = pSystem->setOutput(FMOD_OUTPUTTYPE_WAVWRITER_NRT);
	result = pSystem->init(100, FMOD_INIT_NORMAL | FMOD_INIT_ENABLE_PROFILE, cDest);
	Warning("FMOD: cDest '%s' \n", cDest);

	result = pSystem->createStream(GetFullPathToSound(pathToFileFromSoundsFolder), FMOD_DEFAULT, 0, &pSound);
	//Warning("FMOD: pathToFileFromSoundsFolder '%s' \n", pathToFileFromSoundsFolder);
	//Warning("FMOD: GetFullPathToSound '%s' \n", GetFullPathToSound(pathToFileFromSoundsFolder));
	if (result != FMOD_OK)
	{
		Warning("FMOD: Failed to create stream of sound '%s' ! (ERROR NUMBER: %i)\n", pathToFileFromSoundsFolder, result);
		return;
	}

	result = pSystem->playSound(FMOD_CHANNEL_REUSE, pSound, false, &pChannel);

	if (result != FMOD_OK)
	{
		Warning("FMOD: Failed to play sound '%s' ! (ERROR NUMBER: %i)\n", pathToFileFromSoundsFolder, result);
		return;
	}

	if (fadeIn)
	{
		pChannel->setVolume(0.0);
		m_bFadeIn = true;
	}

	currentSound = pathToFileFromSoundsFolder;
}

// Abruptly stops playing all ambient sounds
void CFMODManager::StopAmbientSound(bool fadeOut)
{
	if (fadeOut)
	{
		pChannel->setVolume(1.0);
		m_bFadeOut = true;
	}
	else
	{
		pChannel->setVolume(0.0);
	}

	currentSound = "NULL";
}

// Transitions between two ambient sounds if necessary
// If a sound isn't already playing when this is called, don't worry about it
void CFMODManager::TransitionAmbientSounds(const char* pathToFileFromSoundsFolder)
{
	pChannel->setVolume(1.0);
	newSoundFileToTransitionTo = pathToFileFromSoundsFolder;

	m_bFadeOut = true;
	m_bShouldTransition = true;
	m_bFadeIn = true;
}
#endif