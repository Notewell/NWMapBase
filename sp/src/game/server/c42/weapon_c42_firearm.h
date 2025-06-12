//Purpose: Baseclass for firearms.

#ifndef _WEAPON_C42FIREARM_C
#define _WEAPON_C42FIREARM_C

#include "in_buttons.h"
#include "ammodef.h"

#ifdef CLIENT_DLL
	#include "flashlighteffect.h"
#endif

#include "hl2_player.h"
#include "weapon_c42_base.h"

#ifdef MAPBASE
extern acttable_t* GetPistolActtable();
extern int GetPistolActtableCount();
#endif

class CWeaponC42FirearmBase : public CBaseC42Weapon
{

	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponC42FirearmBase, CBaseC42Weapon);

	CWeaponC42FirearmBase(void);

	DECLARE_SERVERCLASS();


	void	AddViewKick(void);
	void	DryFire(void);


#ifdef MAPBASE
	void	FireNPCPrimaryAttack(CBaseCombatCharacter* pOperator, Vector& vecShootOrigin, Vector& vecShootDir);
	void	Operator_ForceNPCFire(CBaseCombatCharacter* pOperator, bool bSecondary);
#endif

	void FireProjectile(void);

	void UpdatePenalityTime(void);

	int CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	Activity GetPrimaryAttackActivity(void);

	//virtual bool Reload(void);

	//virtual const Vector& GetBulletSpread(void);
#ifdef MAPBASE
	// Default to pistol acttable
	virtual acttable_t* GetBackupActivityList() { return GetPistolActtable(); }
	virtual int				GetBackupActivityListCount() { return GetPistolActtableCount(); }
#endif


	DECLARE_ACTTABLE();

protected:
	float m_flSoonestPrimaryAttack;
	float m_flLastAttackTime;
	float m_flAccuracyPenalty;
	float m_nNumShotsFired;

	float m_horiztontalviewkickmin = 0.0f;
	float m_horizontalviewkickmax = 0.3f;

	float m_verticalveiwkickmax = 0.6f;
	
	float m_accuracy_max_penalty_time = 1.5f;

};

#endif