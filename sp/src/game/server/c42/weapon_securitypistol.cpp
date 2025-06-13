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

#include "weapon_securitypistol.h"

#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(weapon_securitypistol, CWeaponSecurityPistol);

PRECACHE_WEAPON_REGISTER(weapon_securitypistol);


BEGIN_DATADESC(CWeaponSecurityPistol)
END_DATADESC()

#ifdef MAPBASE
acttable_t	CWeaponSecurityPistol::m_acttable[] =
{
#if EXPANDED_HL2_WEAPON_ACTIVITIES
	{ ACT_IDLE,						ACT_IDLE_REVOLVER,					true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_REVOLVER,				true },
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_REVOLVER,			true },
	{ ACT_RELOAD,					ACT_RELOAD_REVOLVER,					true },
	{ ACT_WALK_AIM,					ACT_WALK_AIM_REVOLVER,				true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_REVOLVER,				true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_REVOLVER,	true },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_REVOLVER_LOW,				false },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_REVOLVER_LOW,		false },
	{ ACT_COVER_LOW,				ACT_COVER_REVOLVER_LOW,				false },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_REVOLVER_LOW,			false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_REVOLVER,			false },
	{ ACT_WALK,						ACT_WALK_REVOLVER,					true },
	{ ACT_RUN,						ACT_RUN_REVOLVER,					true },
#else
	{ ACT_IDLE,						ACT_IDLE_PISTOL,				true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_PISTOL,			true },
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_PISTOL,		true },
	{ ACT_RELOAD,					ACT_RELOAD_PISTOL,				true },
	{ ACT_WALK_AIM,					ACT_WALK_AIM_PISTOL,			true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_PISTOL,				true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_PISTOL,true },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_PISTOL_LOW,			false },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_PISTOL_LOW,	false },
	{ ACT_COVER_LOW,				ACT_COVER_PISTOL_LOW,			false },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_PISTOL_LOW,		false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_PISTOL,		false },
	{ ACT_WALK,						ACT_WALK_PISTOL,				false },
	{ ACT_RUN,						ACT_RUN_PISTOL,					false },
#endif

	// 
	// Activities ported from weapon_alyxgun below
	// 

	// Readiness activities (not aiming)
#if EXPANDED_HL2_WEAPON_ACTIVITIES
	{ ACT_IDLE_RELAXED,				ACT_IDLE_PISTOL_RELAXED,		false },//never aims
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_PISTOL_STIMULATED,		false },
#else
	{ ACT_IDLE_RELAXED,				ACT_IDLE_PISTOL,				false },//never aims
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_STIMULATED,			false },
#endif
	{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_PISTOL,			false },//always aims
	{ ACT_IDLE_STEALTH,				ACT_IDLE_STEALTH_PISTOL,		false },

#if EXPANDED_HL2_WEAPON_ACTIVITIES
	{ ACT_WALK_RELAXED,				ACT_WALK_PISTOL_RELAXED,		false },//never aims
	{ ACT_WALK_STIMULATED,			ACT_WALK_PISTOL_STIMULATED,		false },
#else
	{ ACT_WALK_RELAXED,				ACT_WALK,						false },//never aims
	{ ACT_WALK_STIMULATED,			ACT_WALK_STIMULATED,			false },
#endif
	{ ACT_WALK_AGITATED,			ACT_WALK_AIM_PISTOL,			false },//always aims
	{ ACT_WALK_STEALTH,				ACT_WALK_STEALTH_PISTOL,		false },

#if EXPANDED_HL2_WEAPON_ACTIVITIES
	{ ACT_RUN_RELAXED,				ACT_RUN_PISTOL_RELAXED,			false },//never aims
	{ ACT_RUN_STIMULATED,			ACT_RUN_PISTOL_STIMULATED,		false },
#else
	{ ACT_RUN_RELAXED,				ACT_RUN,						false },//never aims
	{ ACT_RUN_STIMULATED,			ACT_RUN_STIMULATED,				false },
#endif
	{ ACT_RUN_AGITATED,				ACT_RUN_AIM_PISTOL,				false },//always aims
	{ ACT_RUN_STEALTH,				ACT_RUN_STEALTH_PISTOL,			false },

	// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_PISTOL,				false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_ANGRY_PISTOL,			false },
	{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_PISTOL,			false },//always aims
	{ ACT_IDLE_AIM_STEALTH,			ACT_IDLE_STEALTH_PISTOL,		false },

	{ ACT_WALK_AIM_RELAXED,			ACT_WALK,						false },//never aims
	{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_PISTOL,			false },
	{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_PISTOL,			false },//always aims
	{ ACT_WALK_AIM_STEALTH,			ACT_WALK_AIM_STEALTH_PISTOL,	false },//always aims

	{ ACT_RUN_AIM_RELAXED,			ACT_RUN,						false },//never aims
	{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_PISTOL,				false },
	{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_PISTOL,				false },//always aims
	{ ACT_RUN_AIM_STEALTH,			ACT_RUN_AIM_STEALTH_PISTOL,		false },//always aims
	//End readiness activities

	// Crouch activities
	{ ACT_CROUCHIDLE_STIMULATED,	ACT_CROUCHIDLE_STIMULATED,		false },
	{ ACT_CROUCHIDLE_AIM_STIMULATED,ACT_RANGE_AIM_PISTOL_LOW,		false },//always aims
	{ ACT_CROUCHIDLE_AGITATED,		ACT_RANGE_AIM_PISTOL_LOW,		false },//always aims

	// Readiness translations
	{ ACT_READINESS_RELAXED_TO_STIMULATED, ACT_READINESS_PISTOL_RELAXED_TO_STIMULATED, false },
	{ ACT_READINESS_RELAXED_TO_STIMULATED_WALK, ACT_READINESS_PISTOL_RELAXED_TO_STIMULATED_WALK, false },
	{ ACT_READINESS_AGITATED_TO_STIMULATED, ACT_READINESS_PISTOL_AGITATED_TO_STIMULATED, false },
	{ ACT_READINESS_STIMULATED_TO_RELAXED, ACT_READINESS_PISTOL_STIMULATED_TO_RELAXED, false },

#if EXPANDED_HL2_COVER_ACTIVITIES
	{ ACT_RANGE_AIM_MED,			ACT_RANGE_AIM_REVOLVER_MED,			false },
	{ ACT_RANGE_ATTACK1_MED,		ACT_RANGE_ATTACK_REVOLVER_MED,		false },
#endif

#ifdef MAPBASE
	// HL2:DM activities (for third-person animations in SP)
#if EXPANDED_HL2DM_ACTIVITIES
	{ ACT_HL2MP_IDLE,					ACT_HL2MP_IDLE_REVOLVER,                    false },
	{ ACT_HL2MP_RUN,					ACT_HL2MP_RUN_REVOLVER,                    false },
	{ ACT_HL2MP_WALK,					ACT_HL2MP_WALK_REVOLVER,                    false },
	{ ACT_HL2MP_IDLE_CROUCH,			ACT_HL2MP_IDLE_CROUCH_REVOLVER,            false },
	{ ACT_HL2MP_WALK_CROUCH,			ACT_HL2MP_WALK_CROUCH_REVOLVER,            false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,	ACT_HL2MP_GESTURE_RANGE_ATTACK_REVOLVER,    false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK2,	ACT_HL2MP_GESTURE_RANGE_ATTACK2_REVOLVER,    false },
	{ ACT_HL2MP_GESTURE_RELOAD,			ACT_HL2MP_GESTURE_RELOAD_REVOLVER,        false },
	{ ACT_HL2MP_JUMP,					ACT_HL2MP_JUMP_REVOLVER,                    false },
#else
	{ ACT_HL2MP_IDLE,					ACT_HL2MP_IDLE_PISTOL,                    false },
	{ ACT_HL2MP_RUN,					ACT_HL2MP_RUN_PISTOL,                    false },
	{ ACT_HL2MP_IDLE_CROUCH,			ACT_HL2MP_IDLE_CROUCH_PISTOL,            false },
	{ ACT_HL2MP_WALK_CROUCH,			ACT_HL2MP_WALK_CROUCH_PISTOL,            false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,	ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL,    false },
	{ ACT_HL2MP_GESTURE_RELOAD,			ACT_HL2MP_GESTURE_RELOAD_PISTOL,        false },
	{ ACT_HL2MP_JUMP,					ACT_HL2MP_JUMP_PISTOL,                    false },
#endif
#endif
};


IMPLEMENT_ACTTABLE(CWeaponSecurityPistol);

acttable_t* GetSecurityPistolActtable()
{
	return CWeaponSecurityPistol::m_acttable;
}

int GetSecuritPistolActtableCount()
{
	return ARRAYSIZE(CWeaponSecurityPistol::m_acttable);
}
#endif

CWeaponSecurityPistol::CWeaponSecurityPistol(void)
{
	m_bReloadsSingly = false;
	m_bFiresUnderwater = true;
	m_isTrueSemiauto = true;

#ifdef MAPBASE
	m_fMinRange1 = 24;
	m_fMaxRange1 = 1000;
	m_fMinRange2 = 24;
	m_fMaxRange2 = 200;
#endif
}

void CWeaponSecurityPistol::Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_PISTOL_FIRE:
	{
		Vector vecShootOrigin, vecShootDir;
		vecShootOrigin = pOperator->Weapon_ShootPosition();

		CAI_BaseNPC* npc = pOperator->MyNPCPointer();
		ASSERT(npc != NULL);

		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);

#ifdef MAPBASE
		FireNPCPrimaryAttack(pOperator, vecShootOrigin, vecShootDir);
#else
		CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());

		WeaponSound(SINGLE_NPC);
		pOperator->FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2);
		pOperator->DoMuzzleFlash();
		m_iClip1 = m_iClip1 - 1;
#endif
	}
	break;
	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

void CWeaponSecurityPistol::PrimaryAttack(void)
{
	if ((gpGlobals->curtime - m_flLastAttackTime) > 0.5f)
	{
		m_nNumShotsFired = 0;
	}
	else
	{
		m_nNumShotsFired++;
	}
	
	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, GetOwner());
	
	//CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	//
	//if (pOwner)
	//	pOwner->ViewPunchReset(); // This was in the HL2 code for the pistol but maybe we don't wanna do that?
	//// I think the drift might be good for differentiation (and applying player skill)

	BaseClass::PrimaryAttack();

}