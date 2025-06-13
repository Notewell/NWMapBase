
//Purpose: Base weapon for C42, allowing us to define shared behaviors differently from the HLCombatWeapons, while not stepping on toes.

#include "basecombatweapon_shared.h"

#ifndef WEAPON_C42_BASE_H
#define WEAPON_C42_BASE_H
#ifdef _WIN32
#pragma once
#endif

#if defined( CLIENT_DLL )
#define CBaseHLCombatWeapon C_BaseHLCombatWeapon
#endif

class CBaseC42Weapon : public CBaseCombatWeapon
{
#if !defined( CLIENT_DLL )
	DECLARE_DATADESC();

#endif
	DECLARE_CLASS(CBaseC42Weapon, CBaseCombatWeapon);
public:
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual bool	WeaponShouldBeLowered(void);

	bool	CanLower();
	virtual bool	Ready(void);
	virtual bool	Lower(void);
	virtual bool	Deploy(void);
	virtual bool	Holster(CBaseCombatWeapon* pSwitchingTo);
	virtual void	WeaponIdle(void);

	virtual void			PrimaryAttack(void);

	void			ItemPostFrame(void);

	virtual void	AddViewmodelBob(CBaseViewModel* viewmodel, Vector& origin, QAngle& angles);
	virtual	float	CalcViewmodelBob(void);

	virtual Vector	GetBulletSpread(WeaponProficiency_t proficiency);
	virtual float	GetSpreadBias(WeaponProficiency_t proficiency);

	virtual const	WeaponProficiencyInfo_t* GetProficiencyValues();
	static const	WeaponProficiencyInfo_t* GetDefaultProficiencyValues();

	void			HandleFireOnEmpty();

	virtual void	ItemHolsterFrame(void);

#ifdef MAPBASE
	virtual const char* GetViewModel(int viewmodelindex = 0) const;
	virtual float	GetViewmodelFOVOverride() const;
	virtual bool	UsesHands(void) const;
	virtual int		GetHandRig(void) const;
#endif

	int				m_iPrimaryAttacks;		// # of primary attacks performed with this weapon
	int				m_iSecondaryAttacks;	// # of secondary attacks performed with this weapon

protected:

	bool			m_bLowered;			// Whether the viewmodel is raised or lowered
	float			m_flRaiseTime;		// If lowered, the time we should raise the viewmodel
	float			m_flHolsterTime;	// When the weapon was holstered
};


#endif