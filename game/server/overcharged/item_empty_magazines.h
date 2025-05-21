#ifndef	ITEM_EMPTY_MAGAZINES_H
#define	ITEM_EMPTY_MAGAZINES_H

#include "gamerules.h"
#include "player.h"
#include "items.h"


//-----------------------------------------------------------------------------
// Small health kit. Heals the player when picked up.
//-----------------------------------------------------------------------------
class CE_MAG : public CItem
{
public:
	DECLARE_CLASS(CE_MAG, CItem);

	void Spawn(void);
	void Precache(void);
	virtual void VPhysicsCollision(int index, gamevcollisionevent_t *pEvent);

	string_t hitSound;
};

#endif