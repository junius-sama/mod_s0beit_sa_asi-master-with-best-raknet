#include "main.h"

extern struct gui *menu_titlebar_background;
extern struct gui *menu_background;
extern struct gui *menu_selected_item_bar;
extern struct gui *menu_selected_item_text;

extern struct gui *gta_hp_bar;
extern struct gui *gta_money_hud;

TwBar *twBar_MainMenu;
TwBar *twBar_NewCheats;
TwBar *twBar_Aimbot;

void runNewMenu ()
{
	if ( KEY_PRESSED(set.key_new_menu) )
	{
		if ( gta_menu_active() ) return;
		if (g_SAMP != NULL) if (g_Scoreboard->iIsEnabled) return;

		cheat_state->_generic.new_menu ^= 1;
		if ( cheat_state->_generic.new_menu )
			toggleATBCursor( true );
		else
			toggleATBCursor( false );
	}
}

DWORD dwLastUpdate_MenuPopulator;
void menuPopulator ()
{
	if ( dwLastUpdate_MenuPopulator < (GetTickCount() - set.new_menu_populator_time) )
	{
		dwLastUpdate_MenuPopulator = GetTickCount();
	}
}

DWORD menuUpdateHook_origfunc = 0x50B8F0;
void _declspec(naked) menuUpdateHook ()
{
	_asm call menuUpdateHook_origfunc
	_asm pushad
	menuPopulator();
	_asm popad
	_asm retn
}

void TW_CALL newcheatsMenuLinker(void *)
{
	if(TwIsBarMinimized(twBar_NewCheats))
		TwMaximizeBar(twBar_NewCheats);
	else
		TwMinimizeBar(twBar_NewCheats);
}

void TW_CALL aimbotMenuLinker(void *)
{
	if(TwIsBarMinimized(twBar_Aimbot))
		TwMaximizeBar(twBar_Aimbot);
	else
		TwMinimizeBar(twBar_Aimbot);
}

void initializeBarsMenu ()
{
	if ( memcmp_safe( (uint8_t *)0x53EB0D, hex_to_bin("E8DECDFCFF"), 5 ) )
	{
		CDetour api;
		if ( api.Create( (uint8_t *) ( (uint32_t)0x53EB0D ), (uint8_t *)menuUpdateHook, DETOUR_TYPE_CALL_FUNC, 5 ) == 0 )
			Log( "Failed to hook menuUpdateHook." );
	}
	else
		Log( "Failed to hook menuUpdateHook (memcmp)" );

	TwDefine(" TW_HELP visible=false iconified=false ");

	char menuParameters[512];
	/*************************************************************************************************************************************/
	int iMainMenuPosX = pPresentParam.BackBufferWidth / 2 - 400 / 2;
	int iMainMenuPosY = pPresentParam.BackBufferHeight - 275;
	sprintf(menuParameters, " Main_Menu label='[-]           PROJECT FOR SANAL BABALARI           [-]' color='000 00 50' position='%d %d' size='405 235' fontsize='2' iconpos='topright' iconmargin='8 24' valueswidth=150 ", iMainMenuPosX, iMainMenuPosY);
	TwDefine(menuParameters);
	TwAddSeparator(twBar_MainMenu, NULL, NULL);

	/*************************************************************************************************************************************/
	iMainMenuPosX = (pPresentParam.BackBufferWidth / 2) - (405 / 2) - 150;
	iMainMenuPosY = (pPresentParam.BackBufferHeight / 2) - (235 / 2) - 150;
	TwAddButton(twBar_MainMenu, "Special Cheats", newcheatsMenuLinker, NULL, " label='Special Cheats' ");
	sprintf(menuParameters, " New_Cheats label='[-]           SPECIAL CHEATS           [-]' color='0 0 50' position='%d %d' size='405 235' fontsize='2' ", iMainMenuPosX, iMainMenuPosY);
	TwDefine(menuParameters);

	/*************************************************************************************************************************************/
	iMainMenuPosX = (pPresentParam.BackBufferWidth / 2) - (355 / 2) - 25;
	iMainMenuPosY = (pPresentParam.BackBufferHeight / 2) - (255 / 2) - 25;
	TwAddButton(twBar_MainMenu, "Aimbot Settings", aimbotMenuLinker, NULL, " label='Aimbot Settings' ");
	sprintf(menuParameters, " Aimbot label='[-]           AIMBOT SETTINGS           [-]' color='0 0 50' position='%d %d' size='405 235' fontsize='2' ", iMainMenuPosX, iMainMenuPosY);
	TwDefine(menuParameters);
	TwAddSeparator(twBar_MainMenu, NULL, NULL);

	/*************************************************************************************************************************************/





	/* AIMBOT */
	TwAddVarRW(twBar_Aimbot, "DL: Silent Aimbot", TW_TYPE_BOOLCPP, &cheat_state->cheatow.silentaim, " label='DL: Silent Aimbot' ");
	TwAddVarRW(twBar_Aimbot, "DL: Range Hack", TW_TYPE_BOOLCPP, &cheat_state->cheatow.rangehack, " label='DL: Range Hack' ");
	TwAddVarRW(twBar_Aimbot, "DL: No Reload", TW_TYPE_BOOLCPP, &cheat_state->cheatow.noreload, " label='DL: No Reload' ");
	TwAddSeparator(twBar_Aimbot, NULL, NULL);

	TwAddVarRW(twBar_Aimbot, "DL: Ignore Driver", TW_TYPE_BOOLCPP, &cheat_state->cheatow.ignoredriver, " label='DL: Ignore Driver' ");
	TwAddVarRW(twBar_Aimbot, "DL: Ignore Dead", TW_TYPE_BOOLCPP, &cheat_state->cheatow.silentaim_death, " label='DL: Ignore Dead' ");
	TwAddVarRW(twBar_Aimbot, "DL: Ignore Wallshot", TW_TYPE_BOOLCPP, &cheat_state->cheatow.silentaim_wallshot, " label='DL: Ignore Wallshot' ");
	TwAddVarRW(twBar_Aimbot, "DL: Only HS", TW_TYPE_BOOLCPP, &cheat_state->cheatow.onlyhs, " label='DL: Only HS' ");

	TwAddSeparator(twBar_Aimbot, NULL, NULL);

	TwAddVarRW(twBar_Aimbot, "DL: Silent Aim Fov", TW_TYPE_INT32, &cheat_state->cheatow.silentaim_fov, " label='DL: Silent Aimbot Fov' min=0 max=99999999 step=10 ");
	TwAddVarRW(twBar_Aimbot, "DL: Silent Aim Circle", TW_TYPE_BOOLCPP, &cheat_state->cheatow.silentaim_circle, " label='DL: Silent Aim Circle' ");

	TwAddSeparator(twBar_Aimbot, "Render", NULL);

	TwAddVarRW(twBar_Aimbot, "DL: Silent Box", TW_TYPE_BOOLCPP, &cheat_state->cheatow.iBoxESP, " label='DL: Silent Box' ");
	TwAddVarRW(twBar_Aimbot, "DL: Silent Trace", TW_TYPE_BOOLCPP, &cheat_state->cheatow.iTracerLine, " label='DL: Silent Trace' ");
	TwAddVarRW(twBar_Aimbot, "DL: Silent Skeleton", TW_TYPE_BOOLCPP, &cheat_state->cheatow.iSkeletESP, " label='DL: Silent Skeleton' ");


	/* GENERAL CHEATS */
	TwDefine(" 'twBar_NewCheats'/'Auto Mode' opened=true ");
	TwAddVarRW(twBar_NewCheats, "DL: AFK Ghost", TW_TYPE_BOOLCPP, &cheat_state->cheatow.afk, " label='DL: AFK Ghost' ");
	TwAddVarRW(twBar_NewCheats, "DL: Bullet Trace", TW_TYPE_BOOLCPP, &cheat_state->cheatow.bullettrace, " label='DL: Bullet Trace' ");
	TwAddVarRW(twBar_NewCheats, "DL: Anti TP", TW_TYPE_BOOLCPP, &cheat_state->cheatow.antitp, " label='DL: Anti TP' ");
	TwAddVarRW(twBar_NewCheats, "DL: Invisible", TW_TYPE_BOOLCPP, &cheat_state->cheatow.invisible, " label='DL: Invisible' ");
	TwAddVarRW(twBar_NewCheats, "DL: TInvisible", TW_TYPE_BOOLCPP, &cheat_state->cheatow.testinvisible, " label='DL: TInvisible' ");
	TwAddVarRW(twBar_NewCheats, "DL: Take Money", TW_TYPE_BOOLCPP, &cheat_state->cheatow.paraverlanit, " label='DL: Take Money' ");
	TwAddVarRW(twBar_NewCheats, "DL: Godmode", TW_TYPE_BOOLCPP, &cheat_state->_generic.hp_cheat, " label='DL: Godmode' ");
	TwAddSeparator(twBar_NewCheats, NULL, NULL);



	TwDefine(" GLOBAL iconpos=bottomright ");
}

#define FUNC_TOGGLECURSOR			0x63E20
#define FUNC_CURSORUNLOCKACTORCAM	0x63D00
void toggleATBCursor(bool bToggle)
{
	uint32_t    func = g_dwSAMP_Addr + SAMP_FUNC_TOGGLECURSOR;
	uint32_t    funcunlock = g_dwSAMP_Addr + SAMP_FUNC_CURSORUNLOCKACTORCAM;

	uint32_t    obj = *(DWORD*)(g_dwSAMP_Addr + SAMP_MISC_INFO);

	if (bToggle)
	{
		_asm
		{
			mov ecx, obj;
			push 0;
			push 3;
			call func;
		}
	}
	else
	{
		_asm
		{
			mov ecx, obj;
			push 1;
			push 0;
			call func;
		}

		_asm
		{
			mov ecx, obj;
			call funcunlock;
		}
	}
}