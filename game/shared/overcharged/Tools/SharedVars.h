#ifndef SHAREDVARS_H
#define SHAREDVARS_H

#ifdef _WIN32
#pragma once
#endif

#include "convar.h"

inline float timeScale(bool def = false) 
{
	if (def)
		return 1.f;

	float newVal = cvar->FindVar("host_timescale")->GetFloat();

	if (newVal == NULL || newVal <= 0)
		newVal = 1.f;

	return newVal;
}

class GlobalPitch
{

public: float newPitch()
{
	return cvar->FindVar("host_timescale")->GetFloat();
}// = timeScale();//1.f;

};

__declspec(selectany) GlobalPitch *GP = new GlobalPitch();

#endif