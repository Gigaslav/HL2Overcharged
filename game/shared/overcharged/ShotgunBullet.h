
#include "Sprite.h"
#include "SpriteTrail.h"


#define BULLET_MODEL	"models/weapons/flare.mdl"

#define BULLET_AIR_VELOCITY	2500
#define BULLET_WATER_VELOCITY	1500
#define BULLET_SKIN_GLOW		1

class CShotgunBullet : public CBaseEntity
{
	DECLARE_CLASS(CShotgunBullet, CBaseEntity);

public:
	CShotgunBullet() { };
	~CShotgunBullet();

	Class_T Classify(void) { return CLASS_NONE; }

public:
	void Spawn(void);
	void Precache(void);
	void BubbleThink(void);
	void BoltTouch(CBaseEntity *pOther);
	//bool CreateVPhysics(void);
	//unsigned int PhysicsSolidMaskForEntity() const;
	static CShotgunBullet *BoltCreate(const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner = NULL);
	float	m_Speed;



protected:

	bool	CreateSprites(void);

	CHandle<CSprite>		m_pGlowSprite;
	//CHandle<CSpriteTrail>	m_pGlowTrail;

	DECLARE_DATADESC();
	//DECLARE_SERVERCLASS();
};

