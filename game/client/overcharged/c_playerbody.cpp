
#include "cbase.h"
#include "c_playerbody.h"
#include "c_baseanimating.h"
#include "cl_animevent.h"
//#include "c_gstring_player.h"

#include "bone_setup.h"
#include "jigglebones.h"
#include "view.h"
#include "viewrender.h"

ConVar gstring_firstpersonbody_scale("gstring_firstpersonbody_scale", "1");
ConVar gstring_firstpersonbody_hiddenbone_scale("gstring_firstpersonbody_hiddenbone_scale", "0.01");
#if 0 
C_FirstpersonBody::C_FirstpersonBody()
	: m_iBoneNeck(-1)
	, m_iBoneArmL(-1)
	, m_iBoneArmR(-1)
	, m_iPoseParam_MoveYaw(-1)
	, m_bBonescalingEnabled(true)
	, m_vecPlayerOrigin(vec3_origin)
{
}

CStudioHdr *C_FirstpersonBody::OnNewModel()
{
	CStudioHdr *pRet = BaseClass::OnNewModel();

	m_iBoneNeck = LookupBone("ValveBiped.Bip01_Neck1");
	m_iBoneArmL = LookupBone("ValveBiped.Bip01_L_UpperArm");
	m_iBoneArmR = LookupBone("ValveBiped.Bip01_R_UpperArm");
	m_iPoseParam_MoveYaw = LookupPoseParameter("move_yaw");

	return pRet;
}

ShadowType_t C_FirstpersonBody::ShadowCastType()
{
	return SHADOWS_SIMPLE;
}

bool C_FirstpersonBody::ShouldReceiveProjectedTextures(int flags)
{
	return true;
}

void C_FirstpersonBody::BuildTransformations(CStudioHdr *hdr, Vector *pos, Quaternion *q,
	const matrix3x4_t &cameraTransform, int boneMask, CBoneBitList &boneComputed)
{
	if (!hdr)
		return;

	matrix3x4_t bonematrix;
	bool boneSimulated[MAXSTUDIOBONES];

	// no bones have been simulated
	memset(boneSimulated, 0, sizeof(boneSimulated));
	mstudiobone_t *pbones = hdr->pBone(0);

	if (m_pRagdoll)
	{
		// simulate bones and update flags
		int oldWritableBones = m_BoneAccessor.GetWritableBones();
		int oldReadableBones = m_BoneAccessor.GetReadableBones();
		m_BoneAccessor.SetWritableBones(BONE_USED_BY_ANYTHING);
		m_BoneAccessor.SetReadableBones(BONE_USED_BY_ANYTHING);

#if defined( REPLAY_ENABLED )
		// If we're playing back a demo, override the ragdoll bones with cached version if available - otherwise, simulate.
		if ((!engine->IsPlayingDemo() && !engine->IsPlayingTimeDemo()) ||
			!CReplayRagdollCache::Instance().IsInitialized() ||
			!CReplayRagdollCache::Instance().GetFrame(this, engine->GetDemoPlaybackTick(), boneSimulated, &m_BoneAccessor))
#endif
		{
			m_pRagdoll->RagdollBone(this, pbones, hdr->numbones(), boneSimulated, m_BoneAccessor);
		}

		m_BoneAccessor.SetWritableBones(oldWritableBones);
		m_BoneAccessor.SetReadableBones(oldReadableBones);
	}

	// For EF_BONEMERGE entities, copy the bone matrices for any bones that have matching names.
	bool boneMerge = IsEffectActive(EF_BONEMERGE);
	if (boneMerge || m_pBoneMergeCache)
	{
		if (boneMerge)
		{
			if (!m_pBoneMergeCache)
			{
				m_pBoneMergeCache = new CBoneMergeCache;
				m_pBoneMergeCache->Init(this);
			}
			m_pBoneMergeCache->MergeMatchingBones(boneMask);
		}
		else
		{
			delete m_pBoneMergeCache;
			m_pBoneMergeCache = NULL;
		}
	}

	for (int i = 0; i < hdr->numbones(); i++)
	{
		// Only update bones reference by the bone mask.
		if (!(hdr->boneFlags(i) & boneMask))
		{
			continue;
		}

		if (m_pBoneMergeCache && m_pBoneMergeCache->IsBoneMerged(i))
			continue;

		// animate all non-simulated bones
		if (boneSimulated[i] || CalcProceduralBone(hdr, i, m_BoneAccessor))
		{
			continue;
		}
		// skip bones that the IK has already setup
		else if (boneComputed.IsBoneMarked(i))
		{
			// dummy operation, just used to verify in debug that this should have happened
			GetBoneForWrite(i);
		}
		else
		{
			QuaternionMatrix(q[i], pos[i], bonematrix);

			Assert(fabs(pos[i].x) < 100000);
			Assert(fabs(pos[i].y) < 100000);
			Assert(fabs(pos[i].z) < 100000);

			if ((hdr->boneFlags(i) & BONE_ALWAYS_PROCEDURAL) &&
				(hdr->pBone(i)->proctype & STUDIO_PROC_JIGGLE))
			{
				//
				// Physics-based "jiggle" bone
				// Bone is assumed to be along the Z axis
				// Pitch around X, yaw around Y
				//

				// compute desired bone orientation
				matrix3x4_t goalMX;

				if (pbones[i].parent == -1)
				{
					ConcatTransforms(cameraTransform, bonematrix, goalMX);
				}
				else
				{
					ConcatTransforms(GetBone(pbones[i].parent), bonematrix, goalMX);
				}

				// get jiggle properties from QC data
				mstudiojigglebone_t *jiggleInfo = (mstudiojigglebone_t *)pbones[i].pProcedure();

				if (!m_pJiggleBones)
				{
					m_pJiggleBones = new CJiggleBones;
				}

				// do jiggle physics
				m_pJiggleBones->BuildJiggleTransformations(i, gpGlobals->realtime, jiggleInfo, goalMX, GetBoneForWrite(i));

			}
			else if (hdr->boneParent(i) == -1)
			{
				ConcatTransforms(cameraTransform, bonematrix, GetBoneForWrite(i));
			}
			else
			{
				ConcatTransforms(GetBone(hdr->boneParent(i)), bonematrix, GetBoneForWrite(i));
			}
		}

		if (hdr->boneParent(i) == -1)
		{
			MatrixScaleBy(gstring_firstpersonbody_scale.GetFloat(), GetBoneForWrite(i));
		}

		if (m_bBonescalingEnabled
			&& (i == m_iBoneNeck
			|| i == m_iBoneArmR
			|| i == m_iBoneArmL))
		{
			MatrixScaleBy(gstring_firstpersonbody_hiddenbone_scale.GetFloat(), GetBoneForWrite(i));
		}
	}
}

void C_FirstpersonBody::FireEvent(const Vector& origin, const QAngle& angles, int event, const char *options)
{
	switch (event)
	{
	case CL_EVENT_FOOTSTEP_LEFT:
	case CL_EVENT_FOOTSTEP_RIGHT:
	{
		C_GstringPlayer *pPlayer = ToGstringPlayer(C_BasePlayer::GetLocalPlayer());

		if (pPlayer != NULL)
		{
			pPlayer->UpdateStepSoundOverride(pPlayer->GetGroundSurface(), pPlayer->GetAbsOrigin(), pPlayer->GetAbsVelocity());
		}
	}
	break;
	}
}

int C_FirstpersonBody::DrawModel(int flags)
{
	const int viewId = CurrentViewID();

	if (viewId == VIEW_SHADOW_DEPTH_TEXTURE
		&& !C_GstringPlayer::ShouldFirstpersonModelCastShadow()
		|| viewId == VIEW_MONITOR)
	{
		return 0;
	}

	if (viewId == VIEW_REFLECTION)
	{
		const Vector &vecViewOrigin(CurrentViewOrigin());
		VisibleFogVolumeInfo_t fogVolumeInfo;
		render->GetVisibleFogVolume(vecViewOrigin, &fogVolumeInfo);
		if (fogVolumeInfo.m_nVisibleFogVolume >= 0)
		{
			if (fogVolumeInfo.m_flWaterHeight - vecViewOrigin.z > -25.0f)
			{
				return 0;
			}
		}
	}

	if (IsInThirdPersonView())
	{
		m_bBonescalingEnabled = false;
	}

	{
		CBaseAnimating::AutoAllowBoneAccess boneAccess(true, false);

		InvalidateBoneCache();
		SetupBones(NULL, -1, BONE_USED_BY_ANYTHING, gpGlobals->curtime);
	}

	int ret = BaseClass::DrawModel(flags);
	m_bBonescalingEnabled = true;

	return ret;
}

void C_FirstpersonBody::StudioFrameAdvance()
{
	BaseClass::StudioFrameAdvance();

	for (int i = 0; i < GetNumAnimOverlays(); i++)
	{
		C_AnimationLayer *pLayer = GetAnimOverlay(i);

		if (pLayer->m_nSequence < 0)
		{
			continue;
		}

		float rate = GetSequenceCycleRate(GetModelPtr(), pLayer->m_nSequence);

		pLayer->m_flCycle += pLayer->m_flPlaybackRate * rate * gpGlobals->frametime;

		if (pLayer->m_flCycle > 1.0f)
		{
			pLayer->m_nSequence = -1;
			pLayer->m_flWeight = 0.0f;
		}
	}
}

const Vector &C_FirstpersonBody::GetRenderOrigin()
{
	if (!IsCurrentViewIdAccessAllowed())
		return BaseClass::GetRenderOrigin();

	const int viewId = CurrentViewID();

	if (viewId == VIEW_REFLECTION
		|| viewId == VIEW_MONITOR)
		return m_vecPlayerOrigin;

	return BaseClass::GetRenderOrigin();
}

void C_FirstpersonBody::GetRenderBounds(Vector &mins, Vector &maxs)
{
	mins.Init(-32, -32, 0.0f);
	maxs.Init(32, 32, 96.0f);
}

void C_FirstpersonBody::SetPlayerOrigin(const Vector &origin)
{
	m_vecPlayerOrigin = origin;
}

bool C_FirstpersonBody::IsInThirdPersonView()
{
	if (!IsCurrentViewIdAccessAllowed())
		return false;

	const int viewId = CurrentViewID();

	return viewId == VIEW_SHADOW_DEPTH_TEXTURE
		|| viewId == VIEW_REFLECTION
		//|| viewId == VIEW_REFRACTION
		|| viewId == VIEW_MONITOR;
}
#endif