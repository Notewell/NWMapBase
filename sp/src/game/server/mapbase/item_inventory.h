
#ifndef ITEMINVENTORY_H
#define ITEMINVENTORY_H


#include "cbase.h"
#include "gamerules.h"
#include "player.h"
#include "items.h"
#include "in_buttons.h"
#include "engine/IEngineSound.h"

class CItemInventory : public CItem
{
public:
	DECLARE_CLASS(CItemInventory, CItem);

	void Spawn(void);
	void Precache(void);
	bool MyTouch(CBasePlayer* pPlayer);

	char ItemCustomClass;

	DECLARE_DATADESC();

};

	PRECACHE_REGISTER(item_inventory);

#endif
