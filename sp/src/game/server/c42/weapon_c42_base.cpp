//Purpose: Base weapon for C42, allowing us to define shared behaviors differently from the HLCombatWeapons, while not stepping on toes.

#include "cbase.h"
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