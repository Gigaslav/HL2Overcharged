#ifndef C_FIRSTPERSONBODY_H
#define C_FIRSTPERSONBODY_H

#include "c_baseanimatingoverlay.h"
#include "baseentity_shared.h"

#if 0
class C_FirstpersonBody : public C_BaseAnimatingOverlay
{
	DECLARE_CLASS(C_FirstpersonBody, C_BaseAnimatingOverlay);
public:
	C_FirstpersonBody();

	virtual int ObjectCaps() {
		return FCAP_DONT_SAVE;
	};

	virtual void BuildTransformations(CStudioHdr *hdr, Vector *pos, Quaternion *q,
		const matrix3x4_t &cameraTransform, int boneMask, CBoneBitList &boneComputed);

	virtual void FireEvent(const Vector& origin, const QAngle& angles, int event, const char *options);

	virtual CStudioHdr *OnNewModel();

	virtual ShadowType_t ShadowCastType();

	virtual int DrawModel(int flags);

	virtual void StudioFrameAdvance();

	virtual const Vector &GetRenderOrigin();

	void SetPlayerOrigin(const Vector &origin);

	virtual bool ShouldReceiveProjectedTextures(int flags);
	virtual RenderGroup_t GetRenderGroup() { return RENDER_GROUP_OPAQUE_ENTITY; }

	virtual void GetRenderBounds(Vector &mins, Vector &maxs);

	int m_iPoseParam_MoveYaw;

private:
	bool IsInThirdPersonView();

	bool m_bBonescalingEnabled;

	int m_iBoneNeck;
	int m_iBoneArmL;
	int m_iBoneArmR;

	Vector m_vecPlayerOrigin;

};

#endif
#endif