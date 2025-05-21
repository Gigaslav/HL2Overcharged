#ifndef C_GSTRING_PLAYER_RAGDOLL_H
#define C_GSTRING_PLAYER_RAGDOLL_H

#include "c_baseanimating.h"

class C_GStringPlayerRagdoll : public C_ClientRagdoll
{
	DECLARE_CLASS(C_GStringPlayerRagdoll, C_ClientRagdoll);
public:

	C_GStringPlayerRagdoll();

	virtual CStudioHdr *OnNewModel();
	virtual ShadowType_t ShadowCastType() { return SHADOWS_SIMPLE; }
	virtual RenderGroup_t GetRenderGroup() { return RENDER_GROUP_OPAQUE_ENTITY; }

	virtual void BuildTransformations(CStudioHdr *hdr, Vector *pos, Quaternion *q,
		const matrix3x4_t &cameraTransform, int boneMask, CBoneBitList &boneComputed);

private:
	int m_iBoneHead;
};

#endif