#ifndef C_CAM_ATTACHMENT_H
#define C_CAM_ATTACHMENT_H

#include "c_baseanimating.h"


class C_Cam_Attachment : public C_BaseAnimating
{
	DECLARE_CLASS(C_Cam_Attachment, C_BaseAnimating);
public:
	C_Cam_Attachment();
	~C_Cam_Attachment();

	virtual int ObjectCaps() {
		return FCAP_DONT_SAVE;
	};

	virtual bool ShouldDraw() { return true; }
	virtual int DrawModel(int flags) { return 0; }
	virtual void FireEvent(const Vector& origin, const QAngle& angles, int event, const char *options) {}

	virtual CStudioHdr *OnNewModel();

	bool IsDirty() const;
	void SetDirty(bool bDirty);

	bool IsInvalid() const;

	void UpdateDefaultTransforms();
	void GetDeltaTransforms(Vector &posDelta, QAngle &angDelta);

	void SetAttachmentInfo(const FileWeaponInfo_t &info);
	void SetAttachmentInfoMuzzle();//29.12.17
	void SetAttachmentInfoMuzzle2();//29.12.17
	void SetAttachmentInfoShells();//29.12.17
	void SetAttachmentInfoNull();//29.12.17

//private:
	bool GetTransforms(Vector &pos, QAngle &ang);
	int iAttach;
	int iBone;
	float m_flScale;
	int m_iAttachmentFollow;
	int m_iBoneFollow;
private:

	bool m_bDirty;
	bool m_bInvalid;

	QAngle m_angIdle;
	Vector m_posIdle;





	float m_flReferenceCycle;

	QAngle m_angOrientation;
	QAngle m_angOffset;

};


#endif