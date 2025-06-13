
#include "cbase.h"
#include "npcevent.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "te_effect_dispatch.h"
#include "gamestats.h"

#include "weapon_c42_firearm.h"

#include "tier0/memdbgon.h"

#ifndef _SECURITYPISTOL_H
#define _SECURITYPISTOL_H

#ifdef MAPBASE
extern acttable_t* GetPistolActtable();
extern int GetPistolActtableCount();
#endif

class CWeaponSecurityPistol : public CWeaponC42FirearmBase
{
	DECLARE_CLASS(CWeaponSecurityPistol, CWeaponC42FirearmBase);

public:

	CWeaponSecurityPistol(void);

	void PrimaryAttack(void);
	void Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator);

#ifdef MAPBASE
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	virtual int	GetMinBurst() { return 1; }
	virtual int	GetMaxBurst() { return 3; }
	virtual float	GetMinRestTime(void) { return 1.0f; }
	virtual float	GetMaxRestTime(void) { return 2.5f; }

	virtual float GetFireRate(void) { return 0.5f; }

	virtual const Vector& GetBulletSpread(void)
	{
		static Vector cone = VECTOR_CONE_15DEGREES;
		if (!GetOwner() || !GetOwner()->IsNPC())
			return cone;

		static Vector NPCCone = VECTOR_CONE_8DEGREES;
		

		return NPCCone;
	}


	virtual acttable_t* GetBackupActivityList() { return GetPistolActtable(); }
	virtual int				GetBackupActivityListCount() { return GetPistolActtableCount(); }
#endif


	//DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

#ifdef MAPBASE
	DECLARE_ACTTABLE();
#endif

};


#endif