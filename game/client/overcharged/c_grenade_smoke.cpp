#include "cbase.h"
#include "basegrenade_shared.h"
#include "particles_simple.h"
#include "citadel_effects_shared.h"
#include "particles_attractor.h"
#include "iefx.h"
#include "dlight.h"
#include "ClientEffectPrecacheSystem.h"
#include "c_te_effect_dispatch.h"

class C_GrenadeSmoke : public C_BaseGrenade
{
	DECLARE_CLASS( C_GrenadeSmoke, C_BaseGrenade );
	DECLARE_CLIENTCLASS();

	virtual void	UpdateOnRemove( void );
	virtual void	OnDataChanged( DataUpdateType_t type );

	// For RecvProxy handlers

private:

	bool							m_bStartSmoke;
	CNewParticleEffect				*m_hEffect;
};

IMPLEMENT_CLIENTCLASS_DT( C_GrenadeSmoke, DT_GrenadeSmoke, CGrenadeSmoke )
	RecvPropBool( RECVINFO( m_bStartSmoke ) ),
END_RECV_TABLE()

void C_GrenadeSmoke::UpdateOnRemove( void )
{
	if ( m_hEffect )
	{
		m_hEffect->StopEmission();
		m_hEffect = NULL;
	}
}



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_GrenadeSmoke::OnDataChanged( DataUpdateType_t type )
{
	if (m_bStartSmoke)
	{
		m_hEffect = ParticleProp()->Create( cvar->FindVar("oc_weapon_smokegrenade_effect")->GetString() , PATTACH_ABSORIGIN_FOLLOW );
		//m_hEffect->SetControlPointEntity( 0, this );

		m_bStartSmoke = false;
	}
}
