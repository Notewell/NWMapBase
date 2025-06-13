//Purpose: Base weapon for C42, allowing us to define shared behaviors differently from the HLCombatWeapons, while not stepping on toes.

#include "cbase.h"
#include "in_buttons.h"
#include "weapon_c42_base.h"
#ifdef MAPBASE
#include "mapbase/protagonist_system.h"
#endif

#include "hl2_player_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(baseC42weapon, CBaseC42Weapon);

IMPLEMENT_NETWORKCLASS_ALIASED(BaseC42Weapon, DT_BaseC42Weapon)

BEGIN_NETWORK_TABLE(CBaseC42Weapon, DT_BaseC42Weapon)
#if !defined( CLIENT_DLL )
//	SendPropInt( SENDINFO( m_bReflectViewModelAnimations ), 1, SPROP_UNSIGNED ),
#else
//	RecvPropInt( RECVINFO( m_bReflectViewModelAnimations ) ),
#endif
END_NETWORK_TABLE()

#if !defined( CLIENT_DLL )

#include "globalstate.h"

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC(CBaseC42Weapon)

DEFINE_FIELD(m_bLowered, FIELD_BOOLEAN),
DEFINE_FIELD(m_flRaiseTime, FIELD_TIME),
DEFINE_FIELD(m_flHolsterTime, FIELD_TIME),
DEFINE_FIELD(m_iPrimaryAttacks, FIELD_INTEGER),
DEFINE_FIELD(m_iSecondaryAttacks, FIELD_INTEGER),

END_DATADESC()

#endif

//BEGIN_PREDICTION_DATA(CBaseC42Weapon)
//END_PREDICTION_DATA()

void CBaseC42Weapon::ItemHolsterFrame(void)
{
	BaseClass::ItemHolsterFrame();
	// Unlike BaseHLCombatWeapon, we don't auto-reload. Let the player have the thrill of manually reloading
	// each weapon after they get a bunch of ammo they desperately needed between fights.

}

void CBaseC42Weapon::ItemPostFrame(void) // More or less just the baseweapon postframe, with minor changes.
{
	{
		CBasePlayer* pOwner = ToBasePlayer(GetOwner());
		if (!pOwner)
			return;

		UpdateAutoFire(); // Temporarily disabled

		//Track the duration of the fire
		//FIXME: Check for IN_ATTACK2 as well?
		//FIXME: What if we're calling ItemBusyFrame?
		m_fFireDuration = (pOwner->m_nButtons & IN_ATTACK) ? (m_fFireDuration + gpGlobals->frametime) : 0.0f;

		if (UsesClipsForAmmo1())
		{
			CheckReload();
		}

		bool bFired = false;

		// Secondary attack has priority
		if ((pOwner->m_nButtons & IN_ATTACK2) && (m_flNextSecondaryAttack <= gpGlobals->curtime))
		{
#ifdef MAPBASE
			if (pOwner->HasSpawnFlags(SF_PLAYER_SUPPRESS_FIRING))
			{
				// Don't do anything, just cancel the whole function
				return;
			}
			else
#endif
				if (UsesSecondaryAmmo() && pOwner->GetAmmoCount(m_iSecondaryAmmoType) <= 0)
				{
					if (m_flNextEmptySoundTime < gpGlobals->curtime)
					{
						WeaponSound(EMPTY);
						m_flNextSecondaryAttack = m_flNextEmptySoundTime = gpGlobals->curtime + 0.5;
					}
				}
				else if (pOwner->GetWaterLevel() == 3 && m_bAltFiresUnderwater == false)
				{
					// This weapon doesn't fire underwater
					WeaponSound(EMPTY);
					m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
					return;
				}
				else
				{
					// FIXME: This isn't necessarily true if the weapon doesn't have a secondary fire!
					// For instance, the crossbow doesn't have a 'real' secondary fire, but it still 
					// stops the crossbow from firing on the 360 if the player chooses to hold down their
					// zoom button. (sjb) Orange Box 7/25/2007
#if !defined(CLIENT_DLL)
					if (!IsX360() || !ClassMatches("weapon_crossbow"))
#endif
					{
						bFired = ShouldBlockPrimaryFire();
					}

					SecondaryAttack();

					// Secondary ammo doesn't have a reload animation
					if (UsesClipsForAmmo2())
					{
						// reload clip2 if empty
						if (m_iClip2 < 1)
						{
							pOwner->RemoveAmmo(1, m_iSecondaryAmmoType);
							m_iClip2 = m_iClip2 + 1;
						}
					}
				}
		}

		if (!bFired && (pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
		{
#ifdef MAPBASE
			if (pOwner->HasSpawnFlags(SF_PLAYER_SUPPRESS_FIRING))
			{
				// Don't do anything, just cancel the whole function
				return;
			}
			else
#endif
				// Clip empty? Or out of ammo on a no-clip weapon?
				if (!IsMeleeWeapon() &&
					((UsesClipsForAmmo1() && m_iClip1 <= 0) || (!UsesClipsForAmmo1() && pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)))
				{
					HandleFireOnEmpty();
				}
				else if (pOwner->GetWaterLevel() == 3 && m_bFiresUnderwater == false)
				{
					// This weapon doesn't fire underwater
					WeaponSound(EMPTY);
					m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
					return;
				}
				else
				{
					//NOTENOTE: There is a bug with this code with regards to the way machine guns catch the leading edge trigger
					//			on the player hitting the attack key.  It relies on the gun catching that case in the same frame.
					//			However, because the player can also be doing a secondary attack, the edge trigger may be missed.
					//			We really need to hold onto the edge trigger and only clear the condition when the gun has fired its
					//			first shot.  Right now that's too much of an architecture change -- jdw

					// If the firing button was just pressed, or the alt-fire just released, reset the firing time
					if ((pOwner->m_afButtonPressed & IN_ATTACK) || (pOwner->m_afButtonReleased & IN_ATTACK2))
					{
						m_flNextPrimaryAttack = gpGlobals->curtime;
					}

					PrimaryAttack();

					if (AutoFiresFullClip())
					{
						m_bFiringWholeClip = true;
					}

#ifdef CLIENT_DLL
					pOwner->SetFiredWeapon(true);
#endif
				}
		}

		// -----------------------
		//  Reload pressed / Clip Empty
		// -----------------------
		if ((pOwner->m_nButtons & IN_RELOAD) && UsesClipsForAmmo1() && !m_bInReload)
		{
			// reload when reload is pressed
			Reload(); //Reenable ME
			m_fFireDuration = 0.0f;
		}

		// -----------------------
		//  No buttons down
		// -----------------------
		if (!((pOwner->m_nButtons & IN_ATTACK) || (pOwner->m_nButtons & IN_ATTACK2) || (CanReload() && pOwner->m_nButtons & IN_RELOAD)))
		{
			// no fire buttons down or reloading
			if (m_bInReload == false)
			{
				WeaponIdle();
			}
		}
	}

	

}

void CBaseC42Weapon::PrimaryAttack(void)
{
	// If my clip is empty (and I use clips) start reload
	//if (UsesClipsForAmmo1() && !m_iClip1)
	//{
	//	Reload();
	//	return;
	//}

	// Only the player fires this way so we can cast
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	pPlayer->DoMuzzleFlash();

	SendWeaponAnim(GetPrimaryAttackActivity());

	// player "shoot" animation
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	FireBulletsInfo_t info;
	info.m_vecSrc = pPlayer->Weapon_ShootPosition();

	info.m_vecDirShooting = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	info.m_iShots = 0;
	float fireRate = GetFireRate();

	while (m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		// MUST call sound before removing a round from the clip of a CMachineGun
		WeaponSound(SINGLE, m_flNextPrimaryAttack);
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
		info.m_iShots++;
		if (!fireRate)
			break;
	}

	// Make sure we don't fire more than the amount in the clip
	if (UsesClipsForAmmo1())
	{
		info.m_iShots = MIN(info.m_iShots, m_iClip1);
		m_iClip1 -= info.m_iShots;
	}
	else
	{
		info.m_iShots = MIN(info.m_iShots, pPlayer->GetAmmoCount(m_iPrimaryAmmoType));
		pPlayer->RemoveAmmo(info.m_iShots, m_iPrimaryAmmoType);
	}

	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 2;

#if !defined( CLIENT_DLL )
	// Fire the bullets
	info.m_vecSpread = pPlayer->GetAttackSpread(this);
#else
	//!!!HACKHACK - what does the client want this function for? 
	info.m_vecSpread = GetActiveWeapon()->GetBulletSpread();
#endif // CLIENT_DLL

	pPlayer->FireBullets(info);

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	//Add our view kick in
	AddViewKick();
}

void CBaseC42Weapon::HandleFireOnEmpty()
{
	// If we're already firing on empty, reload if we can
	if (m_bFireOnEmpty)
	{
			//ReloadOrSwitchWeapons();
			m_fFireDuration = 0.0f;
	}
	else
	{
		if (m_flNextEmptySoundTime < gpGlobals->curtime)
		{
			WeaponSound(EMPTY);
			m_flNextEmptySoundTime = gpGlobals->curtime + 0.5;
		}
		m_bFireOnEmpty = true;
	}
}

bool CBaseC42Weapon::CanLower()
{
	if (SelectWeightedSequence(ACT_VM_IDLE_LOWERED) == ACTIVITY_NOT_AVAILABLE)
		return false;
	return true;
}

bool CBaseC42Weapon::Lower(void)
{
	//Don't bother if we don't have the animation
	if (SelectWeightedSequence(ACT_VM_IDLE_LOWERED) == ACTIVITY_NOT_AVAILABLE)
		return false;

	m_bLowered = true;
	return true;
}

bool CBaseC42Weapon::Ready(void)
{
	//Don't bother if we don't have the animation
	if (SelectWeightedSequence(ACT_VM_LOWERED_TO_IDLE) == ACTIVITY_NOT_AVAILABLE)
		return false;

	m_bLowered = false;
	m_flRaiseTime = gpGlobals->curtime + 0.5f;
	return true;
}

bool CBaseC42Weapon::Deploy(void)
{
	// If we should be lowered, deploy in the lowered position
	// We have to ask the player if the last time it checked, the weapon was lowered
	if (GetOwner() && GetOwner()->IsPlayer())
	{
		CHL2_Player* pPlayer = assert_cast<CHL2_Player*>(GetOwner());
		if (pPlayer->IsWeaponLowered())
		{
			if (SelectWeightedSequence(ACT_VM_IDLE_LOWERED) != ACTIVITY_NOT_AVAILABLE)
			{
				if (DefaultDeploy((char*)GetViewModel(), (char*)GetWorldModel(), ACT_VM_IDLE_LOWERED, (char*)GetAnimPrefix()))
				{
					m_bLowered = true;

					// Stomp the next attack time to fix the fact that the lower idles are long
					pPlayer->SetNextAttack(gpGlobals->curtime + 1.0);
					m_flNextPrimaryAttack = gpGlobals->curtime + 1.0;
					m_flNextSecondaryAttack = gpGlobals->curtime + 1.0;
					return true;
				}
			}
		}
	}

	m_bLowered = false;
	return BaseClass::Deploy();
}

bool CBaseC42Weapon::Holster(CBaseCombatWeapon* pSwitchingTo)
{
	if (BaseClass::Holster(pSwitchingTo))
	{
		m_flHolsterTime = gpGlobals->curtime;
		return true;
	}

	return false;
}

bool CBaseC42Weapon::WeaponShouldBeLowered(void)
{
	// Can't be in the middle of another animation
	if (GetIdealActivity() != ACT_VM_IDLE_LOWERED && GetIdealActivity() != ACT_VM_IDLE &&
		GetIdealActivity() != ACT_VM_IDLE_TO_LOWERED && GetIdealActivity() != ACT_VM_LOWERED_TO_IDLE)
		return false;

	if (m_bLowered)
		return true;

#if !defined( CLIENT_DLL )

	if (GlobalEntity_GetState("friendly_encounter") == GLOBAL_ON)
		return true;

#endif

	return false;
}

void CBaseC42Weapon::WeaponIdle(void)
{
	//See if we should idle high or low
	if (WeaponShouldBeLowered())
	{
#if !defined( CLIENT_DLL )
		CHL2_Player* pPlayer = dynamic_cast<CHL2_Player*>(GetOwner());

		if (pPlayer)
		{
			pPlayer->Weapon_Lower();
		}
#endif

		// Move to lowered position if we're not there yet
		if (GetActivity() != ACT_VM_IDLE_LOWERED && GetActivity() != ACT_VM_IDLE_TO_LOWERED
			&& GetActivity() != ACT_TRANSITION)
		{
			SendWeaponAnim(ACT_VM_IDLE_LOWERED);
		}
		else if (HasWeaponIdleTimeElapsed())
		{
			// Keep idling low
			SendWeaponAnim(ACT_VM_IDLE_LOWERED);
		}
	}
	else
	{
		// See if we need to raise immediately
		if (m_flRaiseTime < gpGlobals->curtime && GetActivity() == ACT_VM_IDLE_LOWERED)
		{
			SendWeaponAnim(ACT_VM_IDLE);
		}
		else if (HasWeaponIdleTimeElapsed())
		{
			SendWeaponAnim(ACT_VM_IDLE);
		}
	}
}

float g_c42lateralBob;

float g_c42verticalBob;

#if defined(CLIENT_DLL)

//Keep these values seperated from the HL2 ones.
#define	C42_BOB_CYCLE_MIN	1.0f
#define	C42_BOB_CYCLE_MAX	0.45f
#define	C42_BOB			0.002f
#define	C42_BOB_UP		0.5f

static ConVar	cl_bobcycle42("cl_c42_bobcycle", "0.8");
static ConVar	cl_bob42("cl_c42_bob", "0.002");
static ConVar	cl_bobup42("cl_c42_bobup", "0.5");

float CBaseC42Weapon::CalcViewmodelBob(void)
{
	static float bobtime;
	static float lastbobtime;
	float cycle;

	CBasePlayer* player = ToBasePlayer(GetOwner());

	if ((!gpGlobals->frametime) || (player == NULL))
	{
		return 0.0f; //Use previous value
	}

	float speed = player->GetLocalVelocity().Length2D();

	speed = clamp(speed, -320, 320);
	float bob_offset = RemapVal(speed, 0, 320, 0.0f, 1.0f);
	bobtime += (gpGlobals->curtime - lastbobtime) * bob_offset;
	lastbobtime = gpGlobals->curtime;

	//calc vertical bob (we are gonna just blindly trust and replicate Valve's math here)

	cycle = bobtime - (int)(bobtime / C42_BOB_CYCLE_MAX) * C42_BOB_CYCLE_MAX;
	cycle /= C42_BOB_CYCLE_MAX;

	if (cycle < C42_BOB_UP)
	{
		cycle = M_PI * cycle / C42_BOB_UP;
	}
	else
	{
		cycle = M_PI + M_PI * (cycle - C42_BOB_UP) / (1.0 - C42_BOB_UP);
	}

	g_c42verticalBob = speed * 0.005f;
	g_c42verticalBob = g_c42verticalBob * 0.3 + g_c42verticalBob * 0.7 * sin(cycle);

	g_c42verticalBob = clamp(g_c42verticalBob, -7.0f, 4.0f);

	//Calculate the lateral bob
	cycle = bobtime - (int)(bobtime / C42_BOB_CYCLE_MAX * 2) * C42_BOB_CYCLE_MAX * 2;
	cycle /= C42_BOB_CYCLE_MAX * 2;

	if (cycle < C42_BOB_UP)
	{
		cycle = M_PI * cycle / C42_BOB_UP;
	}
	else
	{
		cycle = M_PI + M_PI * (cycle - C42_BOB_UP) / (1.0 - C42_BOB_UP);
	}

	g_c42lateralBob = speed * 0.005f;
	g_c42lateralBob = g_c42lateralBob * 0.3 + g_c42lateralBob * 0.7 * sin(cycle);
	g_c42lateralBob = clamp(g_c42lateralBob, -7.0f, 4.0f);

#ifdef MAPBASE
	if (GetBobScale() != 1.0f)
	{
		//g_verticalBob *= GetBobScale();
		g_c42lateralBob *= GetBobScale();
	}
#endif

	//NOTENOTE: We don't use this return value in our case (need to restructure the calculation function setup!)
	return 0.0f;
}

void CBaseC42Weapon::AddViewmodelBob(CBaseViewModel* viewmodel, Vector& origin, QAngle& angles)
{
	Vector forward, right;
	AngleVectors(angles, &forward, &right, NULL);

	CalcViewmodelBob();

	// Apply bob, but scaled down to 40%
	VectorMA(origin, g_c42verticalBob * 0.1f, forward, origin);

	// Z bob a bit more
	origin[2] += g_c42verticalBob * 0.1f;

	// bob the angles
	angles[ROLL] += g_c42verticalBob * 0.5f;
	angles[PITCH] -= g_c42verticalBob * 0.4f;

	angles[YAW] -= g_c42lateralBob * 0.3f;

	VectorMA(origin, g_c42lateralBob * 0.8f, right, origin);
}

Vector CBaseC42Weapon::GetBulletSpread(WeaponProficiency_t proficiency)
{
	return BaseClass::GetBulletSpread(proficiency);
}

//-----------------------------------------------------------------------------
float CBaseC42Weapon::GetSpreadBias(WeaponProficiency_t proficiency)
{
	return BaseClass::GetSpreadBias(proficiency);
}

const WeaponProficiencyInfo_t* CBaseC42Weapon::GetProficiencyValues()
{
	return NULL;
}

#else

// Server stubs
float CBaseC42Weapon::CalcViewmodelBob(void)
{
	return 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
//			&angles - 
//			viewmodelindex - 
//-----------------------------------------------------------------------------
void CBaseC42Weapon::AddViewmodelBob(CBaseViewModel* viewmodel, Vector& origin, QAngle& angles)
{
}


//-----------------------------------------------------------------------------
Vector CBaseC42Weapon::GetBulletSpread(WeaponProficiency_t proficiency)
{
	Vector baseSpread = BaseClass::GetBulletSpread(proficiency);

	const WeaponProficiencyInfo_t* pProficiencyValues = GetProficiencyValues();
	float flModifier = (pProficiencyValues)[proficiency].spreadscale;
	return (baseSpread * flModifier);
}

//-----------------------------------------------------------------------------
float CBaseC42Weapon::GetSpreadBias(WeaponProficiency_t proficiency)
{
	const WeaponProficiencyInfo_t* pProficiencyValues = GetProficiencyValues();
	return (pProficiencyValues)[proficiency].bias;
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t* CBaseC42Weapon::GetProficiencyValues()
{
	return GetDefaultProficiencyValues();
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t* CBaseC42Weapon::GetDefaultProficiencyValues()
{
	// Weapon proficiency table. Keep this in sync with WeaponProficiency_t enum in the header!!
	static WeaponProficiencyInfo_t g_BaseWeaponProficiencyTable[] =
	{
		{ 2.50, 1.0	},
		{ 2.00, 1.0	},
		{ 1.50, 1.0	},
		{ 1.25, 1.0 },
		{ 1.00, 1.0	},
	};

	COMPILE_TIME_ASSERT(ARRAYSIZE(g_BaseWeaponProficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return g_BaseWeaponProficiencyTable;
}

#endif

#ifdef MAPBASE
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char* CBaseC42Weapon::GetViewModel(int viewmodelindex) const
{
	if (GetOwner() && GetOwner()->IsPlayer() && viewmodelindex == 0)
	{
		const char* pszProtagVM = g_ProtagonistSystem.GetProtagonist_ViewModel(static_cast<CBasePlayer*>(GetOwner()), this);
		if (pszProtagVM)
			return pszProtagVM;
	}

	return BaseClass::GetViewModel(viewmodelindex);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CBaseC42Weapon::GetViewmodelFOVOverride() const
{
	if (GetOwner() && GetOwner()->IsPlayer())
	{
		float* flVMFOV = g_ProtagonistSystem.GetProtagonist_ViewModelFOV(static_cast<CBasePlayer*>(GetOwner()), this);
		if (flVMFOV)
			return *flVMFOV;
	}

	return BaseClass::GetViewmodelFOVOverride();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseC42Weapon::UsesHands() const
{
	if (GetOwner() && GetOwner()->IsPlayer())
	{
		bool* bProtagUsesHands = g_ProtagonistSystem.GetProtagonist_UsesHands(static_cast<CBasePlayer*>(GetOwner()), this);
		if (bProtagUsesHands)
			return *bProtagUsesHands;
	}

	return BaseClass::UsesHands();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseC42Weapon::GetHandRig() const
{
	if (GetOwner() && GetOwner()->IsPlayer())
	{
		int* nProtagHandRig = g_ProtagonistSystem.GetProtagonist_HandRig(static_cast<CBasePlayer*>(GetOwner()), this);
		if (nProtagHandRig)
			return *nProtagHandRig;
	}

	return BaseClass::GetHandRig();
}
#endif