#ifndef	VECTORSTRANSLATE_H
#define	VECTORSTRANSLATE_H

#include "basegrenade_shared.h"

class CParticleSystem;


class CVectorsTranslate : public CBaseAnimating
{
	DECLARE_CLASS(CVectorsTranslate, CBaseAnimating);

public:

	Vector Fwd;
	Vector Right;
	Vector Up;

};

#endif	//GRENADESPIT_H