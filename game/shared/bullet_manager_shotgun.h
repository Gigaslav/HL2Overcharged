// (More) Reallistic simulated bullets
//
// This code is originally based from article on Valve Developer Community
// The original code you can find by this link:
// http://developer.valvesoftware.com/wiki/Simulated_Bullets

#include "ammodef.h"
#include "takedamageinfo.h"

#define BUTTER_MODEShotgun -1
#define ONE_HIT_MODEShotgun -2

#ifdef CLIENT_DLL//-----------------------
class C_BulletManagerShotgun;
extern C_BulletManagerShotgun *g_pBulletManagerShotgun;
#define CBulletManagerShotgun C_BulletManagerShotgun
#define CSimulatedBulletShotgun C_SimulatedBulletShotgun
#else//-----------------------------------
class CBulletManagerShotgun;
extern CBulletManagerShotgun *g_pBulletManagerShotgun;
#endif//----------------------------------

inline CBulletManagerShotgun *BulletManagerShotgun()
{
	return g_pBulletManagerShotgun;
}
static const char *bmsName = "bullet_manager_shotgun";
extern ConVar g_debug_bullets_shotgun;
class CSimulatedBulletShotgun
{
public:
	CSimulatedBulletShotgun();
	CSimulatedBulletShotgun(FireBulletsInfo_t info, int nDamageType, int nAmmoFlags, Vector newdir, CBaseEntity *pInfictor, CBaseEntity *pAdditionalIgnoreEnt,
		bool bTraceHull
#ifndef CLIENT_DLL
		, CBaseEntity *pCaller
#endif
		);
	~CSimulatedBulletShotgun();

	inline float GetBulletSpeedRatioShotgun(void) //The percent of bullet speed 
	{
		return m_flBulletSpeed / m_flInitialBulletSpeed;
	}

	inline bool IsInWorldShotgun(void) const
	{
		if (m_vecOrigin.x >= MAX_COORD_INTEGER) return false;
		if (m_vecOrigin.y >= MAX_COORD_INTEGER) return false;
		if (m_vecOrigin.z >= MAX_COORD_INTEGER) return false;
		if (m_vecOrigin.x <= MIN_COORD_INTEGER) return false;
		if (m_vecOrigin.y <= MIN_COORD_INTEGER) return false;
		if (m_vecOrigin.z <= MIN_COORD_INTEGER) return false;
		return true;
	}
	inline int GetDamageType(void) const
	{
		return m_iDamageType;
	}

	bool StartSolidShotgun(trace_t &ptr); //Exits solid
	bool AllSolidShotgun(trace_t &ptr); //continues in solid
	bool EndSolidShotgun(trace_t &ptr); //Enters solid
	//bool WaterHitShotgun(const Vector &vecStart, const Vector &vecEnd); //Tests water collision

	bool SimulateBulletShotgun(void); //Main function of bullet simulation

	void HandleShotImpactingWaterShotgun(const FireBulletsInfo_t &info, const Vector &vecStart, const Vector &vecEnd, ITraceFilter *pTraceFilter, Vector *pVecTracerDest, bool isPlayer);


	CBaseEntity *pInfictorRemember;
	Vector m_vecDirShootingRemember;
	CTakeDamageInfo *info2;
	int AmmoType;
	float flActualDamage;
	float flActualDamageRemember;
	float flActualForce;
	//bool IsPlayer;

	void EntityImpactShotgun(trace_t &ptr);	//Impact test

	void TraceAttackToTriggersShotgun(const CTakeDamageInfo &info, const Vector& start, const Vector& end, const Vector& dir, trace_t &tr);

	inline int GetDamageTypeShotgun(void) const
	{
		return m_iDamageType;
	}

	FireBulletsInfo_t bulletinfo;

	inline int GetAmmoTypeIndexShotgun(void) const
	{
		return bulletinfo.m_iAmmoType;
	}

	Vector Point;
	int EntIndexX;
	int entIndex;
private:
	trace_t trEnd;
	Vector EndPoint;
	bool isPlayer;
	bool isUnderWaterShotgun;
	bool m_bTraceHull;	//Trace hull?
	bool m_bWasInWater;
	CAmmoDef *pAmmoDef;
	CTraceFilterSimpleList *m_pIgnoreList; //already hit
#ifndef CLIENT_DLL
	CUtlVector<CBaseEntity *> m_CompensationConsiderations; //Couldn't resist
#endif

	EHANDLE m_hCaller;
	EHANDLE	m_hLastHit;		//Last hit (I think it doesn't work)


	float m_flBulletSpeed;  //The changeable bullet speed
	float m_flEntryDensity; //Sets when doing penetration test
	float m_flInitialBulletSpeed;
	float m_flRayLength;

	float DesiredDistance; //Sets when doing penetration test

	bool m_bPenetrated;

	int m_iDamageType;

	Vector m_vecDirShooting;   //The bullet direction with applied spread
	Vector m_vecOrigin;		   //Bullet origin

	Vector m_vecEntryPosition; //Sets when doing penetration test

	Vector m_vecTraceRay;

	Vector AbsEntry;
	Vector AbsExit;

	CBaseEntity *p_eInfictor;
private:
	bool bStuckInWall; // Indicates bullet 
};

extern CUtlLinkedList<CSimulatedBulletShotgun*> g_BulletsShotgun; //Bullet list


class CBulletManagerShotgun : public CBaseEntity
{
	DECLARE_CLASS(CBulletManagerShotgun, CBaseEntity);
public:
	~CBulletManagerShotgun()
	{
		g_BulletsShotgun.PurgeAndDeleteElements();
	}
	int AddBulletShotgun(CSimulatedBulletShotgun *pBullet);
#ifdef CLIENT_DLL
	void ClientThink(void);
#else
	void Think(void);
	void SendTraceAttackToTriggersShotgun(const CTakeDamageInfo &info, const Vector& start, const Vector& end, const Vector& dir);
#endif

	void RemoveBulletShotgun(int index);	//Removes bullet.
	void UpdateBulletStopSpeedShotgun(void);	//Updates bullet speed

	int BulletStopSpeedShotgun(void) //returns speed that the bullet must be removed
	{
		return m_iBulletStopSpeed;
	}

	CNetworkVector(EndPoint);

private:
	int m_iBulletStopSpeed;
};