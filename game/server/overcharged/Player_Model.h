#ifndef PM_H
#define PM_H
#pragma once

#include "cbase.h"
//#include "hl2_player_shared.h"


class CPlayerModel : public CBaseFlex//CBaseFlex
{
public:
	DECLARE_CLASS(CPlayerModel, CBaseFlex);
	DECLARE_DATADESC();

	virtual void Think(void);
	virtual void Spawn(void);
	virtual void Precache(void);
	virtual void SetOrigin(const Vector &origin, const QAngle &origAngle);
private:
	Vector newOrigin;
	bool inCrouch;
	float posZ;
};
#endif