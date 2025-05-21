// (More) Reallistic simulated bullets
//
// This code is originally based from article on Valve Developer Community
// The original code you can find by this link:
// http://developer.valvesoftware.com/wiki/Simulated_Bullets

#include "ammodef.h"
#include "takedamageinfo.h"

#define BUTTER_MODE -1
#define ONE_HIT_MODE -2

#ifdef CLIENT_DLL//-----------------------
#include "iefx.h"
#include "dlight.h"
class C_BulletManager;
extern C_BulletManager *g_pBulletManager;
#define CBulletManager C_BulletManager
#define CSimulatedBullet C_SimulatedBullet
#else//-----------------------------------
class CBulletManager;

extern CBulletManager *g_pBulletManager;
#endif//----------------------------------

inline CBulletManager *BulletManager()
{

	return g_pBulletManager;
}

static const char *bmName = "bullet_manager";

extern ConVar g_debug_bullets;
class CSimulatedBullet
{
	
public:
	DECLARE_CLASS(CSimulatedBullet, CSimulatedBullet);

	//DECLARE_CLASS(CSimulatedBullet, CBaseEntity);
	CSimulatedBullet(int nEntIndex);//int nEntIndex = 0
	CSimulatedBullet(FireBulletsInfo_t info, int nDamageType, int nAmmoFlags, Vector newdir, CBaseEntity *pInfictor, CBaseEntity *pAdditionalIgnoreEnt,
		bool bTraceHull
#ifndef CLIENT_DLL
		, CBaseEntity *pCaller
#endif
		);
	~CSimulatedBullet();

	inline float GetBulletSpeedRatio(void) //The percent of bullet speed 
	{
		return m_flBulletSpeed / m_flInitialBulletSpeed;
	}

	inline bool IsInWorld(void) const
	{
		if (m_vecOrigin.x >= MAX_COORD_INTEGER) return false;
		if (m_vecOrigin.y >= MAX_COORD_INTEGER) return false;
		if (m_vecOrigin.z >= MAX_COORD_INTEGER) return false;
		if (m_vecOrigin.x <= MIN_COORD_INTEGER) return false;
		if (m_vecOrigin.y <= MIN_COORD_INTEGER) return false;
		if (m_vecOrigin.z <= MIN_COORD_INTEGER) return false;
		return true;
	}

	bool StartSolid(trace_t &ptr); //Exits solid
	bool AllSolid(trace_t &ptr); //continues in solid
	bool EndSolid(trace_t &ptr); //Enters solid
	//bool WaterHit(const Vector &vecStart, const Vector &vecEnd); //Tests water collision
	// Handle shot entering water
	void HandleShotImpactingWater(const FireBulletsInfo_t &info, const Vector &vecStart, const Vector &vecEnd, ITraceFilter *pTraceFilter, Vector *pVecTracerDest, bool isPlayer);

	bool SimulateBullet(void); //Main function of bullet simulation

	CBaseEntity *pInfictorRemember;
	Vector m_vecDirShootingRemember;
	CTakeDamageInfo *info2;
	int AmmoType;
	float flActualDamage;
	float flActualDamageRemember;
	float flActualForce;

	//void CalculateBulletDamageForce2(CTakeDamageInfo *info, int iBulletType, const Vector &vecBulletDir, const Vector &vecForceOrigin, float flScale = 1.0);
	// (CBaseEntity *pInfictorRemember, CBaseEntity p_eInfictor, float flActualDamage, int Damage);

	bool EntityImpact(trace_t &ptr);	//Impact test

	void TraceAttackToTriggers(const CTakeDamageInfo &info, const Vector& start, const Vector& end, const Vector& dir, trace_t &tr);

	inline int GetDamageType(void) const
	{
		return m_iDamageType;
	}

	FireBulletsInfo_t pFireBulletsInfo;

	inline int GetAmmoTypeIndex(void) const
	{
		return pFireBulletsInfo.m_iAmmoType;
	}

	Vector Point;
	int EntIndexX;
	int entIndex;
private:
	bool penetrated;

	float penetrationLength;
	float lifeAfterImpact;

	//bool IsPlayer;
	trace_t trEnd;
	Vector EndPoint;
	bool isPlayer;
	bool isUnderWater;
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
	int TracerType;
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

extern CUtlLinkedList<CSimulatedBullet*> g_Bullets; //Bullet list


class CBulletManager : public CBaseEntity
{
	DECLARE_CLASS(CBulletManager, CBaseEntity);
public:
	~CBulletManager()
	{
		g_Bullets.PurgeAndDeleteElements();
	}
	int AddBullet(CSimulatedBullet *pBullet);
#ifdef CLIENT_DLL
	void ClientThink(void);
#else
	void Think(void);
	void SendTraceAttackToTriggers(const CTakeDamageInfo &info, const Vector& start, const Vector& end, const Vector& dir);
	void Delete() { UTIL_Remove(g_pBulletManager); }

#endif
	void RemoveBullet(int index);	//Removes bullet.
	void UpdateBulletStopSpeed(void);	//Updates bullet speed

	int BulletStopSpeed(void) //returns speed that the bullet must be removed
	{
		return m_iBulletStopSpeed;
	}


	//DECLARE_SERVERCLASS();
	//DECLARE_DATADESC();




	//CNetworkVarForDerived(int, index);
	//CNetworkVarForDerived(Vector, EndPoint);
	CNetworkVector(EndPoint);
	CNetworkVar(int, index);


private:
	int m_iBulletStopSpeed;
};



