#ifndef c_tracer_filter
#define c_tracer_filter
#ifdef _WIN32
#pragma once
#endif



class CTraceFilterSkipPlayerAndViewModel : public CTraceFilter
{
public:
	virtual bool ShouldHitEntity(IHandleEntity *pServerEntity, int contentsMask)
	{
		// Test against the vehicle too?
		// FLASHLIGHTFIXME: how do you know that you are actually inside of the vehicle?
		C_BaseEntity *pEntity = EntityFromEntityHandle(pServerEntity);
		if (!pEntity)
			return true;

		if ((dynamic_cast<C_BaseViewModel *>(pEntity) != NULL) ||
			(dynamic_cast<C_BasePlayer *>(pEntity) != NULL) ||
			pEntity->GetCollisionGroup() == COLLISION_GROUP_DEBRIS ||
			pEntity->GetCollisionGroup() == COLLISION_GROUP_INTERACTIVE_DEBRIS)
		{
			return false;
		}

		return true;
	}
};

#endif