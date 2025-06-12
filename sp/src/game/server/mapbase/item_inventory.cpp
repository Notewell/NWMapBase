
#include "cbase.h"
#include "gamerules.h"
#include "player.h"
#include "items.h"
#include "in_buttons.h"
#include "engine/IEngineSound.h"
#include "item_inventory.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


LINK_ENTITY_TO_CLASS(item_inventory, CItemInventory);
PRECACHE_REGISTER(item_healthkit);

BEGIN_DATADESC(CItemInventory)

END_DATADESC()

void CItemInventory::Spawn()
{
	BaseClass::Spawn();

}

void CItemInventory::Precache(void)
{
	BaseClass::Precache();
}

bool CItemInventory::MyTouch(CBasePlayer* pPlayer)
{
	if (pPlayer)
	{
		pPlayer->m_rgItems[ItemCustomClass] += 1;
		return true;
	}
	return false;
}

