//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: Combine guard effects
//
// $NoKeywords: $
//=============================================================================
#include "cbase.h"
#include "c_ai_basenpc.h"
#include "c_te_particlesystem.h"
#include "iefx.h"
#include "dlight.h"
#include "glow_overlay.h"
#include "fx.h"
#include "fx_sparks.h"
#include "basehlcombatweapon_shared.h"
#include "particlemgr.h"
#include "particle_prototype.h"
#include "particle_util.h"
#include "view_scene.h"
#include "c_tracer.h"
#include "c_weapon__stubs.h"
#include "ClientEffectPrecacheSystem.h"
#include "c_basehlcombatweapon.h"

#define CGUARD_MSG_SHOT			1
#define CGUARD_MSG_SHOT_START	2


class CSTParticle : public TrailParticle
{
public:
	float	m_speed;
};


#define	MAX_SPHERE_DIRS		128

static Vector g_SphereDirs[MAX_SPHERE_DIRS];
static int g_SphereDirIndex = -1;

void InitSphereDirs()
{
	if (g_SphereDirIndex >= 0)
		return;

	g_SphereDirIndex = 0;
	for (int i = 0; i < MAX_SPHERE_DIRS; i++)
	{
		float len = 0;
		do
		{
			g_SphereDirs[i].Random(-100, 100);
			len = VectorNormalize(g_SphereDirs[i]);
		} while (len < 1);
	}
}

inline const Vector &GetNextSphereDir()
{
	g_SphereDirIndex = (g_SphereDirIndex + 1) & (MAX_SPHERE_DIRS - 1);
	return g_SphereDirs[g_SphereDirIndex];
}


//
// CCombineGuardParticleEffect
//

class CCombineGuardParticleEffect : public CParticleEffect
{
	DECLARE_CLASS(CCombineGuardParticleEffect, CParticleEffect);
public:
	CCombineGuardParticleEffect(const char *pDebugName, const Vector &origin) :
		CParticleEffect(pDebugName)
	{
		SetSortOrigin(origin);
		SetDynamicallyAllocated(true);
	}
	virtual void SimulateParticles(CParticleSimulateIterator *pIterator)
	{
		//BaseClass::SimulateParticles(pIterator);
	}
	void RenderParticles(CParticleRenderIterator *pIterator)
	{
		//BaseClass::RenderParticles(pIterator);
	}
private:
	CCombineGuardParticleEffect(const CCombineGuardParticleEffect &); // not defined, not accessible
};

//
// CCombineGuardBallParticle
//

class CCombineGuardBallParticle : public Particle
{
public:
	float t;
};

#define BALL_GROW_TIME	0.9f
#define BALL_RELEASE_TIME	0.1f

//
// CCombineGuardBall
//

class CCombineGuardBall : public CCombineGuardParticleEffect
{
	DECLARE_CLASS(CCombineGuardBall, CCombineGuardParticleEffect);
public:

	CCombineGuardBall(const char *pDebugName, const Vector &origin, float radius, float speed, float delay, int entityIndex, int attachment)
		: CCombineGuardParticleEffect(pDebugName, origin), m_radius(radius), m_effectSpeed(1 / speed),
		m_entityIndex(entityIndex), m_attachment(attachment)
	{
		SetSortOrigin(origin);
		m_boneOrigin = origin;
		PMaterialHandle centerMaterial = GetPMaterial("sprites/strider_blackball");
		CCombineGuardBallParticle *pBall = (CCombineGuardBallParticle *)AddParticle(sizeof(CCombineGuardBallParticle), centerMaterial, origin);

		if (pBall)
		{
			pBall->t = -delay;
		}
		m_clock = 0;
		m_dieTime = (BALL_GROW_TIME + BALL_RELEASE_TIME) * m_effectSpeed;
		m_vecColor.Init(1, 1, 1);
	}
	bool SimulateAndRender(Particle *pInParticle, ParticleDraw *pDraw, float &sortKey)
	{
		CCombineGuardBallParticle *pParticle = (CCombineGuardBallParticle *)pInParticle;
		float timeDelta = pDraw->GetTimeDelta();

		if (pParticle->t < BALL_GROW_TIME || IsReleased())
		{
			//Should this particle die?
			pParticle->t += timeDelta;

			if (pParticle->t >= m_dieTime)
				return false;
			// not visible yet, but don't kill
			if (pParticle->t < 0)
				return true;
		}

		float t = pParticle->t - BALL_GROW_TIME*m_effectSpeed;
		float size;
		if (t > 0)
		{
			size = Bias(1, 0.2) * m_radius;
			if (IsReleased())
			{
				if (t > BALL_RELEASE_TIME*m_effectSpeed)
					t = BALL_RELEASE_TIME*m_effectSpeed;
				t = (BALL_RELEASE_TIME*m_effectSpeed - t) * (1 / BALL_RELEASE_TIME*m_effectSpeed);
				size *= t;
			}
		}
		else
		{
			size = Bias(pParticle->t * (1 / (BALL_GROW_TIME*m_effectSpeed)), 0.2) * m_radius;
		}

		Vector tPos;
		pParticle->m_Pos = m_boneOrigin;
		TransformParticle(ParticleMgr()->GetModelView(), pParticle->m_Pos, tPos);
		sortKey = (int)tPos.z;

		//Render it
		RenderParticle_ColorSize(pDraw, tPos, m_vecColor, 1.0, size);

		return true;
	}

	void StartRender()
	{
		if (m_entityIndex > 0)
		{
			QAngle angles;
			C_BaseEntity *ent = cl_entitylist->GetEnt(m_entityIndex);
			if (ent)
			{
				ent->GetAttachment(m_attachment, m_boneOrigin, angles);
			}
		}
	}
	void Update(float dt)
	{
		m_clock += dt;
		SetSortOrigin(m_boneOrigin);
		dlight_t *dl = effects->CL_AllocDlight(m_entityIndex);
		dl->origin = m_boneOrigin;
		dl->color.r = 40;
		dl->color.g = 60;
		dl->color.b = 250;
		dl->color.exponent = 5;
		float size = (m_clock > m_dieTime) ? m_dieTime : m_clock;
		dl->radius = m_radius * 3 * (size / m_dieTime);
		dl->die = gpGlobals->curtime + 0.001;
	}

	Vector	m_boneOrigin;
	Vector	m_vecColor;
	float	m_radius;
	float	m_effectSpeed;
	int		m_entityIndex;
	int		m_attachment;
	float	m_dieTime;
	float	m_clock;

private:
	CCombineGuardBall(const CCombineGuardBall &);
};

//
// Sphere drawing
//

double noise1(double arg)
{
	return (arg + random->RandomFloat(-1.5f, 1.5f));
}

static void RandomizeNormal(Vector &vec)
{
	vec.x = 2.0f * (noise1(vec.x) - 0.5f);
	vec.y = 2.0f * (noise1(vec.y) - 0.5f);
	vec.z = 2.0f * (noise1(vec.z) - 0.5f);
}

static void DrawSphere(int stacks, int slices, float radius, Vector &origin)
{
	// this sucks and stuff
	float x = origin.x;
	float y = origin.y;
	float z = origin.z;
	float stackAngle, sliceAngle;
	int stack, slice;
	Vector v[4];
	float	sliced, stacked, sliced1, stacked1, stacks1;
	float	slicedsin, slicedcos, stackedsin, stackedcos;
	float	sliced1sin, sliced1cos, stacked1sin, stacked1cos;
	float	stacks1sin, stacks1cos;
	float	stacksin, stackcos;

	CMatRenderContextPtr pRenderContext(materials);
	IMesh *pMesh = pRenderContext->GetDynamicMesh();
	CMeshBuilder meshBuilder;

	stackAngle = M_PI / (float)stacks;
	sliceAngle = 2.0 * M_PI / (float)slices;

	for (stack = 1; stack < stacks - 1; stack++)
	{
		for (slice = 0; slice < slices; slice++)
		{
			int i, j;

			sliced = sliceAngle * slice;
			stacked = stackAngle * stack;
			sliced1 = sliceAngle * (slice + 1);
			stacked1 = stackAngle * (stack + 1);

			SinCos(sliced, &slicedsin, &slicedcos);
			SinCos(stacked, &stackedsin, &stackedcos);
			SinCos(sliced1, &sliced1sin, &sliced1cos);
			SinCos(stacked1, &stacked1sin, &stacked1cos);

			v[0][0] = -slicedsin * stackedsin;
			v[0][1] = slicedcos * stackedsin;
			v[0][2] = stackedcos;

			v[1][0] = -sliced1sin * stackedsin;
			v[1][1] = sliced1cos * stackedsin;
			v[1][2] = stackedcos;

			v[2][0] = -sliced1sin * stacked1sin;
			v[2][1] = sliced1cos * stacked1sin;
			v[2][2] = stacked1cos;

			v[3][0] = -slicedsin * stacked1sin;
			v[3][1] = slicedcos * stacked1sin;
			v[3][2] = stacked1cos;

			for (i = 0; i < 4; i++)
			{
				for (j = 0; j < 3; j++)
				{
					v[i][j] *= radius;
				}
				v[i][0] += x;
				v[i][1] += y;
				v[i][2] += z;
			}

#if 1
			//			if( drawWireframe.value )
			if (1)
			{
				meshBuilder.Begin(pMesh, MATERIAL_QUADS, 1);

				meshBuilder.Position3fv(v[0].Base());
				Vector normal;
				normal = v[0] - origin;
				VectorNormalize(normal);
				RandomizeNormal(normal);
				VectorNormalize(normal);
				meshBuilder.Normal3fv(normal.Base());
				meshBuilder.AdvanceVertex();

				meshBuilder.Position3fv(v[1].Base());
				normal = v[1] - origin;
				VectorNormalize(normal);
				RandomizeNormal(normal);
				VectorNormalize(normal);
				meshBuilder.Normal3fv(normal.Base());
				meshBuilder.AdvanceVertex();

				meshBuilder.Position3fv(v[2].Base());
				normal = v[2] - origin;
				VectorNormalize(normal);
				RandomizeNormal(normal);
				VectorNormalize(normal);
				meshBuilder.Normal3fv(normal.Base());
				meshBuilder.AdvanceVertex();

				meshBuilder.Position3fv(v[3].Base());
				normal = v[3] - origin;
				VectorNormalize(normal);
				RandomizeNormal(normal);
				VectorNormalize(normal);
				meshBuilder.Normal3fv(normal.Base());
				meshBuilder.AdvanceVertex();

				meshBuilder.End();
				pMesh->Draw();
			}
			else
			{
				//				DrawIndexedQuad( v, 0, 1, 2, 3 );
			}
#endif
		}
	}

	// do the caps
	for (slice = 0; slice < slices; slice++)
	{
		int i, j;

		sliced = sliceAngle * slice;
		stacked = stackAngle * stack;
		sliced1 = sliceAngle * (slice + 1);
		stacked1 = stackAngle * (stack + 1);
		stacks1 = stackAngle * (stacks - 1);

		SinCos(sliced, &slicedsin, &slicedcos);
		SinCos(stacked, &stackedsin, &stackedcos);
		SinCos(sliced1, &sliced1sin, &sliced1cos);
		SinCos(stacked1, &stacked1sin, &stacked1cos);
		SinCos(stackAngle, &stacksin, &stackcos);
		SinCos(stacks1, &stacks1sin, &stacks1cos);

		v[0][0] = 0.0f;
		v[0][1] = 0.0f;
		v[0][2] = 1.0f;

		v[1][0] = -sliced1sin * stacksin;
		v[1][1] = sliced1cos * stacksin;
		v[1][2] = stackcos;

		v[2][0] = -slicedsin * stacksin;
		v[2][1] = slicedcos * stacksin;
		v[2][2] = stackcos;

		for (i = 0; i < 3; i++)
		{
			for (j = 0; j < 3; j++)
			{
				v[i][j] *= radius;
			}
			v[i][0] += x;
			v[i][1] += y;
			v[i][2] += z;
		}

		meshBuilder.Begin(pMesh, MATERIAL_TRIANGLES, 1);

		meshBuilder.Position3fv(v[0].Base());
		Vector normal;
		normal = v[0] - origin;
		VectorNormalize(normal);
		RandomizeNormal(normal);
		VectorNormalize(normal);
		meshBuilder.Normal3fv(normal.Base());
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3fv(v[1].Base());
		normal = v[1] - origin;
		VectorNormalize(normal);
		RandomizeNormal(normal);
		VectorNormalize(normal);
		meshBuilder.Normal3fv(normal.Base());
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3fv(v[2].Base());
		normal = v[2] - origin;
		VectorNormalize(normal);
		RandomizeNormal(normal);
		VectorNormalize(normal);
		meshBuilder.Normal3fv(normal.Base());
		meshBuilder.AdvanceVertex();

		meshBuilder.End();
		pMesh->Draw();

		v[0][0] = 0.0f;
		v[0][1] = 0.0f;
		v[0][2] = -1.0f;

		v[1][0] = -sliced1sin * stacks1sin;
		v[1][1] = sliced1cos * stacks1sin;
		v[1][2] = stacks1cos;

		v[2][0] = -slicedsin * stacks1sin;
		v[2][1] = slicedcos * stacks1sin;
		v[2][2] = stacks1cos;

		for (i = 0; i < 3; i++)
		{
			for (j = 0; j < 3; j++)
			{
				v[i][j] *= radius;
			}
			v[i][0] += x;
			v[i][1] += y;
			v[i][2] += z;
		}

		meshBuilder.Begin(pMesh, MATERIAL_TRIANGLES, 1);

		meshBuilder.Position3fv(v[0].Base());
		normal = v[0] - origin;
		VectorNormalize(normal);
		RandomizeNormal(normal);
		VectorNormalize(normal);
		meshBuilder.Normal3fv(normal.Base());
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3fv(v[2].Base());
		normal = v[2] - origin;
		VectorNormalize(normal);
		RandomizeNormal(normal);
		VectorNormalize(normal);
		meshBuilder.Normal3fv(normal.Base());
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3fv(v[1].Base());
		normal = v[1] - origin;
		VectorNormalize(normal);
		RandomizeNormal(normal);
		VectorNormalize(normal);
		meshBuilder.Normal3fv(normal.Base());
		meshBuilder.AdvanceVertex();

		meshBuilder.End();
		pMesh->Draw();
	}
}

class CCombineGuardMuzzleOverlay : public CWarpOverlay
{
public:
	virtual bool Update(void)
	{
		m_flLifetime += gpGlobals->frametime;

		if (m_flLifetime < m_flTotalLifetime)
		{
			return true;
		}

		return false;
	}

	virtual void Draw(void)
	{
		if (m_entityIndex > 0)
		{
			QAngle angles;
			C_BaseEntity *ent = cl_entitylist->GetEnt(m_entityIndex);

			if (ent)
			{
				ent->GetAttachment(m_nAttachment, m_vPos, angles);
			}
		}

		UpdateRefractTexture();
		CMatRenderContextPtr pRenderContext(materials);
		pRenderContext->Bind(m_pWarpMaterial, NULL);
		float maxRadius = 128.0f;
		float radius;
		float fractionOfLifetime = m_flLifetime / m_flTotalLifetime;

		if (fractionOfLifetime < 0.5f)
		{
			radius = fractionOfLifetime * 2.0f * maxRadius;
		}
		else
		{
			radius = (1.0f - fractionOfLifetime) * 2.0f * maxRadius;
		}

		DrawSphere(8, 8, radius, m_vPos);
	}

public:

	int		m_entityIndex;
	int		m_nAttachment;
	IMaterial *m_pWarpMaterial;
	float	m_flLifetime;
	float	m_flTotalLifetime;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_WeaponCguard : public C_BaseHLCombatWeapon
{
	DECLARE_CLASS(C_WeaponCguard, C_BaseHLCombatWeapon);
public:
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	C_WeaponCguard();
				~C_WeaponCguard();

	void			ClientThink(void);

	bool			m_bEmitStreaks;

	CSmartPtr<CSphereTrails> m_pGunTrails;
	CSmartPtr<CCombineGuardBall> m_pGunBall;

	float			m_flTrailTime;
	bool EnableSphere;
	// model specific
	//virtual void	ReceiveMessage(const char *msgname, int length, void *data);
	virtual void	ReceiveMessage(int classID, bf_read &msg);
//private:
	//C_WeaponCguard(const C_WeaponCguard &);
};

STUB_WEAPON_CLASS_IMPLEMENT(weapon_cguard, C_WeaponCguard);

IMPLEMENT_CLIENTCLASS_DT(C_WeaponCguard, DT_WeaponCguard, CWeaponCguard)
RecvPropBool(RECVINFO(EnableSphere)),
END_RECV_TABLE()

C_WeaponCguard::C_WeaponCguard(void)
{
	m_bEmitStreaks = false;
	m_flTrailTime = 0.0f;

	m_pGunTrails = NULL;
	m_pGunBall = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Strider class implementation
//-----------------------------------------------------------------------------
C_WeaponCguard::~C_WeaponCguard()
{
}

int count = 0;
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_WeaponCguard::ClientThink(void)
{
	if (m_bEmitStreaks)
	{
		m_pGunBall = NULL;
		m_pGunTrails = NULL;
		m_bEmitStreaks = false;
	}

	if (EnableSphere)
	{
		int	attachment = LookupAttachment("muzzle");
		m_bEmitStreaks = true;
		SetNextClientThink(gpGlobals->curtime + 0.5f);

		//m_pGunTrails = new CSphereTrails("C_CombineGuard::ReceiveMessage 1", GetAbsOrigin(), 0, 128, 256, entindex(), attachment);
		if (!m_pGunBall)
			m_pGunBall = new CCombineGuardBall("C_CombineGuard::ReceiveMessage 2", GetAbsOrigin(), 32, 1, 0, entindex(), attachment);
		count++;
		m_pGunBall->m_dieTime = 15;
		//Make the warp overlay
		/*if (CCombineGuardMuzzleOverlay *pOverlay = new CCombineGuardMuzzleOverlay)
		{
			pOverlay->m_nAttachment = attachment;
			pOverlay->m_entityIndex = entindex();
			pOverlay->m_vPos = GetAbsOrigin();
			pOverlay->m_flLifetime = 0;
			pOverlay->m_flTotalLifetime = 2.0f;  // seconds
			pOverlay->m_pWarpMaterial = materials->FindMaterial("sprites/predator", TEXTURE_GROUP_CLIENT_EFFECTS);
			pOverlay->Activate();
		}*/
	}
	/*if (count > 100)
		m_pGunBall->m_dieTime*/
	//SetNextClientThink(0.0f);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : length - 
//			*data - 
//-----------------------------------------------------------------------------
void C_WeaponCguard::ReceiveMessage(int classID, bf_read &msg) //ReceiveMessage(const char *msgname, int length, void *data)
{
	if (classID != GetClientClass()->m_ClassID)
	{
		// message is for subclass
		BaseClass::ReceiveMessage(classID, msg);
		return;
	}

	int messageType = msg.ReadByte();
	switch (messageType)
	{
	case CGUARD_MSG_SHOT_START:
	{
		int	attachment = LookupAttachment("muzzle");
		m_bEmitStreaks = true;
		SetNextClientThink(gpGlobals->curtime + 0.5f);

		//m_pGunTrails = new CSphereTrails("C_CombineGuard::ReceiveMessage 1", GetAbsOrigin(), 0, 128, 256, entindex(), attachment);
		m_pGunBall = new CCombineGuardBall("C_CombineGuard::ReceiveMessage 2", GetAbsOrigin(), 32, 1, 0, entindex(), attachment);

		//Make the warp overlay
		if (CCombineGuardMuzzleOverlay *pOverlay = new CCombineGuardMuzzleOverlay)
		{
			pOverlay->m_nAttachment = attachment;
			pOverlay->m_entityIndex = entindex();
			pOverlay->m_vPos = GetAbsOrigin();
			pOverlay->m_flLifetime = 0;
			pOverlay->m_flTotalLifetime = 2.0f;  // seconds
			pOverlay->m_pWarpMaterial = materials->FindMaterial("sprites/predator", TEXTURE_GROUP_CLIENT_EFFECTS);
			pOverlay->Activate();
		}
	}
	break;

	case CGUARD_MSG_SHOT:
	{
		Vector	pos, muzzleDir;
		QAngle	angle;

		GetAttachment(LookupAttachment("muzzle"), pos, angle);

		AngleVectors(angle, &muzzleDir);

		CSmartPtr<CSimpleEmitter> emitter = CSimpleEmitter::Create("CGUARD_MSG_SHOT");
		emitter->SetSortOrigin(pos);

		SimpleParticle	*pParticle;

		for (int i = 0; i < 16; i++)
		{
			pParticle = (SimpleParticle *)emitter->AddParticle(sizeof(SimpleParticle), emitter->GetPMaterial("particle/particle_smokegrenade"), pos);

			pParticle->m_flLifetime = 0.0f;
			pParticle->m_flDieTime = 0.5f;

			pParticle->m_vecVelocity = muzzleDir;
			pParticle->m_vecVelocity[0] += random->RandomFloat(-0.8f, 0.8f);
			pParticle->m_vecVelocity[1] += random->RandomFloat(-0.8f, 0.8f);
			pParticle->m_vecVelocity[2] += random->RandomFloat(-0.8f, 0.8f);
			pParticle->m_vecVelocity *= random->RandomFloat(64, 256);

			pParticle->m_uchColor[0] = pParticle->m_uchColor[1] = pParticle->m_uchColor[2] = 64.0f;

			pParticle->m_uchStartSize = random->RandomInt(16, 32);
			pParticle->m_uchEndSize = pParticle->m_uchStartSize * 2;

			pParticle->m_uchStartAlpha = random->RandomInt(32, 64);
			pParticle->m_uchEndAlpha = 0;

			pParticle->m_flRoll = random->RandomInt(0, 360);
			pParticle->m_flRollDelta = random->RandomFloat(-8.0f, 8.0f);
		}
	}
	break;
	}
}
