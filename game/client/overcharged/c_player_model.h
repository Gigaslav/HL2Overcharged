#if !defined( C_PLAYER_MODEL_H )
#define C_PLAYER_MODEL_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

class C_Player_Model : public C_BaseAnimating
{
	typedef C_BaseAnimating BaseClass;
public:


	~C_Player_Model(void);

	C_Player_Model	*CreateClientsideGib(const Vector &vecOrigin);

	bool	InitializeGib(const Vector &vecOrigin);
	void	ClientThink(void);
	void	StartTouch(C_BaseEntity *pOther);

	virtual	void HitSurface(C_BaseEntity *pOther);

protected:

	float	m_flTouchDelta;		// Amount of time that must pass before another touch function can be called
};


#endif // C_CORPSE_H