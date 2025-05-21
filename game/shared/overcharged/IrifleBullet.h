
#include "Sprite.h"
#include "SpriteTrail.h"


#define IBULLET_MODEL	"models/weapons/flare.mdl"

#define IBULLET_AIR_VELOCITY	2500
#define IBULLET_WATER_VELOCITY	1500
#define IBULLET_SKIN_GLOW		1

class CIrifleBullet : public CBaseEntity
{
	DECLARE_CLASS(CIrifleBullet, CBaseEntity);

public:
	CIrifleBullet() { };
	~CIrifleBullet();

	Class_T Classify(void) { return CLASS_NONE; }

public:
	void Spawn(CBasePlayer *pPlayer = NULL);
	void Precache(void);
	//void BubbleThink(void);
	//void BoltTouch(CBaseEntity *pOther);
	//bool CreateVPhysics(void);
	//unsigned int PhysicsSolidMaskForEntity() const;
	static CIrifleBullet *BoltCreate(/*const Vector &vecOrigin, const QAngle &angAngles, */CBasePlayer *pentOwner = NULL);
	//CHandle<CSpriteTrail>	m_pGlowTrail;

	DECLARE_DATADESC();
	//DECLARE_SERVERCLASS();
};

