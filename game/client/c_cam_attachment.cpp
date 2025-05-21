
#include "cbase.h"
#include "c_cam_attachment.h"

C_Cam_Attachment::C_Cam_Attachment()

	: m_bDirty(false)
	, m_angIdle(vec3_angle)
	, m_posIdle(vec3_origin)
	, m_bInvalid(false)
	, m_iBoneFollow(-1)
	, m_iAttachmentFollow(0)//29.12.17//1
	, m_flScale(1.0f)
	, m_flReferenceCycle(0.0f)
	, m_angOrientation(vec3_angle)
	, m_angOffset(vec3_angle)

{
	iAttach = 1;
}

C_Cam_Attachment::~C_Cam_Attachment()
{
	m_bDirty = false;
	m_angIdle = vec3_angle;
	m_posIdle = Vector(0,0,0);
	m_bInvalid = false;
	m_iBoneFollow = -1;
	m_iAttachmentFollow = 0;
	m_flScale = 1.0f;
	m_flReferenceCycle = 0.0f;
	m_angOrientation = vec3_angle;
	m_angOffset = vec3_angle;
	iAttach = 1;
}

CStudioHdr *C_Cam_Attachment::OnNewModel()
{
	m_bDirty = true;

	return BaseClass::OnNewModel();
}

bool C_Cam_Attachment::IsDirty() const
{
	return m_bDirty;
}

void C_Cam_Attachment::SetDirty(bool bDirty)
{
	m_bDirty = bDirty;
}

bool C_Cam_Attachment::IsInvalid() const
{
	return m_bInvalid;
}

ConVar oc_adjust_camera("oc_adjust_camera", "0");
ConVar oc_adjust_camera_attachment("oc_adjust_camera_attachment", "-1");
ConVar oc_adjust_camera_bone("oc_adjust_camera_bone", "-1");
ConVar oc_adjust_camera_attach_scale("oc_adjust_camera_attach_scale", "0");

extern ConVar oc_player_animated_camera_scale;
void C_Cam_Attachment::UpdateDefaultTransforms()
{
	if (oc_adjust_camera.GetInt())
	{
		iBone = 1;
		m_iBoneFollow = oc_adjust_camera_bone.GetInt();

		iAttach = 1;

		m_iAttachmentFollow = oc_adjust_camera_attachment.GetInt();

		m_flScale = oc_adjust_camera_attach_scale.GetFloat();
	}
	else
	{
		int idx = 0;

		if (GetActiveWeapon()->GetWpnData().AttachmentsArray.Count() > 0)
		{
			iAttach = RandomInt(0, GetActiveWeapon()->GetWpnData().AttachmentsArray.Count() - 1);
			m_iAttachmentFollow = GetActiveWeapon()->GetWpnData().AttachmentsArray.Element(iAttach);
			idx = iAttach;
		}
		else
			m_iAttachmentFollow = -1;

		if (GetActiveWeapon()->GetWpnData().BonesArray.Count() > 0)
		{
			iBone = RandomInt(0, GetActiveWeapon()->GetWpnData().BonesArray.Count()-1) ;
			m_iBoneFollow = GetActiveWeapon()->GetWpnData().BonesArray.Element(iBone);
			idx = iBone;
		}
		else
			m_iBoneFollow = -1;

		if (GetActiveWeapon()->GetWpnData().Scales.Count() > 0)
		{
			idx = Clamp(idx, 0, GetActiveWeapon()->GetWpnData().Scales.Count() - 1);
			int randomScale = GetActiveWeapon()->GetWpnData().Randomize ? (RandomInt(0, GetActiveWeapon()->GetWpnData().Scales.Count() - 1)) : idx;
			m_flScale = GetActiveWeapon()->GetWpnData().Scales.Element(randomScale);//(iBone);
		}
		else
			m_flScale = 1.f;
	}


	m_flScale *= oc_player_animated_camera_scale.GetFloat();

	int iSequence = SelectWeightedSequence(ACT_VM_IDLE);

	/*if (iSequence < 0)
	{
		m_angIdle = vec3_angle;
		m_posIdle = vec3_origin;

		m_bInvalid = true;
		return;
	}*/

	SetSequence(iSequence);
	SetCycle(m_flReferenceCycle);

	Vector vecPos;
	QAngle ang;

	if (GetTransforms(vecPos, ang))
	{
		m_posIdle = vecPos;
		m_angIdle = ang;
		m_bInvalid = false;
	}
	else
	{
		m_bInvalid = true;
	}
}

//Main point
void C_Cam_Attachment::GetDeltaTransforms(Vector &posDelta, QAngle &angDelta)
{
	Assert(!m_bInvalid);

	Vector pos;
	posDelta = m_posIdle;
	GetTransforms(posDelta, angDelta);

	for (int i = 0; i < 3; i++)
	{
		angDelta[i] = AngleDiff(angDelta[i], m_angIdle[i]);
	}
	
	angDelta *= m_flScale;
}
//Second point
bool C_Cam_Attachment::GetTransforms(Vector &pos, QAngle &ang)
{
	matrix3x4_t mat;

	if (m_iBoneFollow >= 0)
	{
		if (!GetModelPtr()
			|| GetModelPtr()->numbones() <= m_iBoneFollow)
			return false;

		SetupBones(NULL, -1, BONE_USED_BY_ANYTHING, gpGlobals->curtime);

		mat = GetBone(m_iBoneFollow);
	}
	else
	{
		if (!GetModelPtr()
			|| GetModelPtr()->GetNumAttachments() < m_iAttachmentFollow)
			return false;

		if (!GetAttachment(m_iAttachmentFollow, mat))
			return false;
	}

	Vector f, r, u;
	MatrixAngles(mat, ang, pos);
	AngleVectors(ang, &f, &r, &u);

	matrix3x4_t rot, tmp;
	MatrixBuildRotationAboutAxis(f, m_angOrientation.x, rot);
	ConcatTransforms(rot, mat, tmp);
	MatrixBuildRotationAboutAxis(r, m_angOrientation.y, rot);
	ConcatTransforms(rot, tmp, mat);
	MatrixBuildRotationAboutAxis(u, m_angOrientation.z, rot);
	ConcatTransforms(rot, mat, tmp);

	MatrixAngles(tmp, ang, pos);
	return true;
}

void C_Cam_Attachment::SetAttachmentInfo(const FileWeaponInfo_t &info)
{
	m_iBoneFollow = LookupBone(info.szCameraBoneName);
	m_iAttachmentFollow = LookupAttachment(info.szCameraAttachmentName);


	if (m_iBoneFollow < 0
		&& m_iAttachmentFollow <= 0)
	{
		m_iAttachmentFollow = 1;
	}

	m_flScale = info.flCameraMovementScale;
	m_flReferenceCycle = info.flCameraMovementReferenceCycle;

	m_angOrientation = info.angCameraMovementOrientation;
	m_angOffset = info.angCameraMovementOffset;

}

void C_Cam_Attachment::SetAttachmentInfoMuzzle()//29.12.17
{
	m_iAttachmentFollow = LookupAttachment("muzzle");//аттачмент к 1-My стволу
}

void C_Cam_Attachment::SetAttachmentInfoMuzzle2()//29.12.17
{
	m_iAttachmentFollow = LookupAttachment("muzzle2");//аттачмент к второму стволу
}

void C_Cam_Attachment::SetAttachmentInfoShells()//29.12.17
{
	m_iAttachmentFollow = LookupAttachment("1");//аттачмент к гильзам
}

void C_Cam_Attachment::SetAttachmentInfoNull()//29.12.17
{
	m_iAttachmentFollow = 0;//аттачмент к гильзам
}