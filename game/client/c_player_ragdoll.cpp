
#include "cbase.h"
#include "c_player_ragdoll.h"

#include "bone_setup.h"
#include "jigglebones.h"
#include "viewrender.h"


C_GStringPlayerRagdoll::C_GStringPlayerRagdoll()
	: BaseClass(false)
	, m_iBoneHead(-1)
{
}

CStudioHdr *C_GStringPlayerRagdoll::OnNewModel()
{
	CStudioHdr *ret = BaseClass::OnNewModel();

	m_iBoneHead = LookupBone("ValveBiped.Bip01_Head1");

	return ret;
}

void C_GStringPlayerRagdoll::BuildTransformations(CStudioHdr *hdr, Vector *pos, Quaternion *q,
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
		if (i == m_iBoneHead)
		{
			MatrixScaleBy(0.01f, GetBoneForWrite(i));
		}

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
	}
}

