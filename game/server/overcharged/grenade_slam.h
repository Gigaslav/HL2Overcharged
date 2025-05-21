//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose:		Satchel Charge
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================

#ifndef	SATCHEL_H
#define	SATCHEL_H

#ifdef _WIN32
#pragma once
#endif


#include "basegrenade_shared.h"
#include "weapon_slam.h"

class CSoundPatch;
class CSprite;

class CSatchelCharge : public CBaseGrenade
{
public:
	DECLARE_CLASS(CSatchelCharge, CBaseGrenade);

	void			Spawn(void);
	void			Precache(void);
	void			BounceSound(void);
	void			SatchelTouch(CBaseEntity *pOther);
	void			SatchelThink(void);

	// Input handlers
	void			InputExplode(inputdata_t &inputdata);

	float			m_flNextBounceSoundTime;
	bool			m_bInAir;
	Vector			m_vLastPosition;

public:
	CWeapon_SLAM*	m_pMyWeaponSLAM;	// Who shot me..
	bool			m_bIsAttached;
	void			Deactivate(void);

	CSatchelCharge();
	~CSatchelCharge();

	DECLARE_DATADESC();

private:
	void				CreateEffects(void);
	CHandle<CSprite>	m_hGlowSprite;
};

#endif	//SATCHEL_H
