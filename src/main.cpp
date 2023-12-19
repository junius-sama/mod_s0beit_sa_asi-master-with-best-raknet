/*

	PROJECT:		mod_sa
	LICENSE:		See LICENSE in the top level directory
	COPYRIGHT:		Copyright we_sux, BlastHack

	mod_sa is available from https://github.com/BlastHackNet/mod_s0beit_sa/

	mod_sa is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	mod_sa is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with mod_sa.  If not, see <http://www.gnu.org/licenses/>.

*/
#include "main.h"


HINSTANCE				g_hOrigDll = NULL;
HMODULE					g_hDllModule = NULL;
char					g_szWorkingDirectory[MAX_PATH];
FILE					*g_flLog = NULL;
FILE					*g_flLogAll = NULL;
FILE					*g_flLogChatbox = NULL;
FILE					*g_flLogChatboxAll = NULL;
char					g_szLastFunc[256];
uint32_t				g_dwSAMP_Addr = NULL;
char					g_szSAMPVer[16];

CSettingsSAInterface	*g_pCSettingsSAInterface = (CSettingsSAInterface *)CLASS_CMenuManager;
D3DPRESENT_PARAMETERS	*g_pGTAPresent = (D3DPRESENT_PARAMETERS *)0xC9C040;
RsGlobalType			*g_RsGlobal = (RsGlobalType *)0xC17040;

// new MTA main global variables
CGameSA					*pGameInterface = NULL;
CPools					*pPools = NULL;
CPed					*pPedSelf = NULL;
CPedSAInterface			*pPedSelfSA = NULL;

// to store information about the Windows OS
t_WindowsInfo			WindowsInfo;

// Orig exception filter
LPTOP_LEVEL_EXCEPTION_FILTER OrigExceptionFilter;

std::string replaceStr(std::string orig_str, const std::string& replace_string, const std::string& with_string) {
	size_t pos = orig_str.find(replace_string);
	if (pos != std::string::npos)
		orig_str.replace(pos, replace_string.length(), with_string);
	return orig_str;
}

void GetAimingCenter(float& x, float& y)
{
	if (g_Players->pLocalPlayer->byteCurrentWeapon != 34)
	{
		x = (float)pPresentParam.BackBufferWidth * 0.5299999714f;
		y = (float)pPresentParam.BackBufferHeight * 0.4f;
	}
	else
	{
		x = (float)pPresentParam.BackBufferWidth / 2.f;
		y = (float)pPresentParam.BackBufferHeight / 2.f;
	}
	return;
}

bool isInsideCircle(float circle_x, float circle_y,
	float rad, float x, float y)
{
	if ((x - circle_x) * (x - circle_x) +
		(y - circle_y) * (y - circle_y) <= rad * rad)
		return true;
	else
		return false;
}

bool IsLineOfSightClear(CVector* vecStart, CVector* vecEnd, bool bCheckBuildings,
	bool bCheckVehicles, bool bCheckPeds, bool bCheckObjects,
	bool bCheckDummies, bool bSeeThroughStuff, bool bIgnoreSomeObjectsForCamera)
{
	DWORD dwFunc = FUNC_IsLineOfSightClear;
	bool bReturn = false;
	// bool bCheckBuildings = true, bool bCheckVehicles = true, bool bCheckPeds = true, 
	// bool bCheckObjects = true, bool bCheckDummies = true, bool bSeeThroughStuff = false, 
	// bool bIgnoreSomeObjectsForCamera = false

	_asm
	{
		push	bIgnoreSomeObjectsForCamera
		push	bSeeThroughStuff
		push	bCheckDummies
		push	bCheckObjects
		push	bCheckPeds
		push	bCheckVehicles
		push	bCheckBuildings
		push	vecEnd
		push	vecStart
		call	dwFunc
		mov		bReturn, al
		add		esp, 0x24
	}
	return bReturn;
}

CVector calculateAngle(const CVector& src, const CVector& dst)
{
	CVector angle;
	CVector delta = src - dst;

	float hyp = sqrtf(delta.fX * delta.fX + delta.fY * delta.fY);

	angle.fX = atanf(delta.fZ / hyp) * (180.0f / M_PI);
	angle.fY = atanf(delta.fY / delta.fY) * (180.0f / M_PI);
	angle.fZ = 0;

	if (delta.fX >= 0.0) angle.fY += 180.0f;

	return angle;
}

float getHeadingFromVector2d(float x, float y)
{
	return 360.0f - atan2(y, x);
}

std::string replaceAll(std::string subject, const std::string& search,
	const std::string& replace) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	};
	return subject;
};

std::string DownloadURL(std::string URL) {
	HINTERNET interwebs = InternetOpenA("Mozilla/5.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL);
	HINTERNET urlFile;
	std::string rtn;
	if (interwebs) {
		urlFile = InternetOpenUrlA(interwebs, URL.c_str(), NULL, NULL, NULL, NULL);
		if (urlFile) {
			char buffer[2000];
			DWORD bytesRead;
			do {
				InternetReadFile(urlFile, buffer, 2000, &bytesRead);
				rtn.append(buffer, bytesRead);
				memset(buffer, 0, 2000);
			} while (bytesRead);
			InternetCloseHandle(interwebs);
			InternetCloseHandle(urlFile);
			std::string p = replaceAll(rtn, "|n", "\r\n");
			return p;
		};
	};
	InternetCloseHandle(interwebs);
	std::string p = replaceAll(rtn, "|n", "\r\n");
	return p;
};

int silentAimTarget()
{
	if (g_SAMP == NULL) return -1;
	int targetid = -1;
	D3DXVECTOR3 ReversePosedion, ReverseEkranPozisyonu;
	float        centerX, centerY;
	CVector2D EkranPozisyonu;

	GetAimingCenter(centerX, centerY);
	float  FFNeah = 0.0;
	for (int i = 0; i < 1004; i++)
	{
		if (g_Players->iIsListed[i] != 1)
			continue;
		if (g_Players->pRemotePlayer[i] == NULL)
			continue;
		if (g_Players->pRemotePlayer[i]->pPlayerData == NULL || g_Players->pRemotePlayer[i]->pPlayerData->pSAMP_Actor == NULL)
			continue;

		float Posedion[3], tarpos[3], mypos[3];
		getPlayerPos(i, Posedion);
		Posedion[2] += 0.35;

		ReversePosedion.x = Posedion[0];
		ReversePosedion.y = Posedion[1];
		ReversePosedion.z = Posedion[2];
		CalcScreenCoors(&ReversePosedion, &ReverseEkranPozisyonu);

		EkranPozisyonu.fX = ReverseEkranPozisyonu.x;
		EkranPozisyonu.fY = ReverseEkranPozisyonu.y;

		getPlayerPos(i, tarpos);
		CVector enemy; enemy.fX = tarpos[0], enemy.fY = tarpos[1], enemy.fZ = tarpos[2];
		getPlayerPos(g_Players->sLocalPlayerID, mypos);
		CVector myself; myself.fX = mypos[0], myself.fY = mypos[1], myself.fZ = mypos[2];
		bool IsPlayerVisible = pGame->GetWorld()->IsLineOfSightClear(pPedSelf->GetPosition(), &enemy, true, !g_Players->pRemotePlayer[i]->pPlayerData->bytePlayerState == 19, false, true, true, false, false);

		double GGNeah = sqrt(((GetSystemMetrics(SM_CXSCREEN) * 0.5299999714f - EkranPozisyonu.fX) * (GetSystemMetrics(SM_CXSCREEN) * 0.5299999714f - EkranPozisyonu.fX)) + ((GetSystemMetrics(SM_CYSCREEN) * 0.4f - EkranPozisyonu.fY) * (GetSystemMetrics(SM_CYSCREEN) * 0.4f - EkranPozisyonu.fY))); // Bakış açışını hesaplar
		if (GGNeah > FFNeah && FFNeah != 0.0) continue;
		if ((g_Players->pRemotePlayer[i]->pPlayerData->bytePlayerState == PLAYER_STATE_DRIVER) && cheat_state->cheatow.ignoredriver) continue;
		if (cheat_state->cheatow.Friends[i]) continue;
		if (g_Players->pRemotePlayer[i]->pPlayerData->iAFKState == 2) continue;
		if (cheat_state->cheatow.silentaim_wallshot && !IsPlayerVisible) continue;
		if (cheat_state->cheatow.silentaim_fov != 0) { if (!isInsideCircle(centerX, centerY, cheat_state->cheatow.silentaim_fov, EkranPozisyonu.fX, EkranPozisyonu.fY)) continue; }
		if ((g_Players->pRemotePlayer[i]->pPlayerData->onFootData.sCurrentAnimationID == 1701 || cheat_state->cheatow.playerdeadanim[i]) && cheat_state->cheatow.silentaim_death) continue;

		FFNeah = (float)GGNeah;
		targetid = i;

		if (cheat_state->cheatow.silentaim_circle && cheat_state->cheatow.silentaim_fov != 0)
			render->CircleOutlined(centerX, centerY, (float)cheat_state->cheatow.silentaim_fov, 42, 1.25, D3DCOLOR_ARGB(255, 255, 0, 0), D3DCOLOR_ARGB(255, 0, 0, 0));
	}
	if (cheat_state->cheatow.aimedplayer != -1)
	{
		actor_info* player = getGTAPedFromSAMPPlayerID(cheat_state->cheatow.aimedplayer);
		if (!player)
			return -1;
		CPed* pPed = pGameInterface->GetPools()->GetPed((DWORD*)player);
		if (!pPed)
			return -1;
		float
			max_up_val = 0,
			max_down_val = 0,
			max_left_val = 0,
			max_right_val = 0;
		bool invalid = false;
		for (int bone_id = BONE_PELVIS1; bone_id <= BONE_RIGHTFOOT; ++bone_id)
		{
			if (bone_id >= BONE_PELVIS1 && bone_id <= BONE_HEAD ||
				bone_id >= BONE_RIGHTUPPERTORSO && bone_id <= BONE_RIGHTTHUMB ||
				bone_id >= BONE_LEFTUPPERTORSO && bone_id <= BONE_LEFTTHUMB ||
				bone_id >= BONE_LEFTHIP && bone_id <= BONE_LEFTFOOT ||
				bone_id >= BONE_RIGHTHIP && bone_id <= BONE_RIGHTFOOT)
			{
				CVector vHead, vFoot;
				pPed->GetBonePosition((eBone)BONE_HEAD, &vHead);
				pPed->GetBonePosition((eBone)BONE_LEFTFOOT, &vFoot);

				D3DXVECTOR3 headPos{ vHead.fX, vHead.fY, vHead.fZ }, footPos{ vFoot.fX, vFoot.fY, vFoot.fZ };
				D3DXVECTOR3 headScreenPos, footScreenPos;


				CalcScreenCoors(&headPos, &headScreenPos);
				CalcScreenCoors(&footPos, &footScreenPos);


				if (headScreenPos.z < 1.f || footScreenPos.z < 1.f)
					continue;


				float tmp = pow((headScreenPos.x - footScreenPos.x), 2);
				float tmp2 = pow((headScreenPos.y - footScreenPos.y), 2);

				float height = (sqrt(tmp + tmp2)) * 1.2f;
				float width = height / 2.0f;

				if (cheat_state->cheatow.iBoxESP && cheat_state->cheatow.silentaim)
				{
					render->D3DBoxBorder(headScreenPos.x - (width / 2.0f), headScreenPos.y - (height / 30.0f), width, height, D3DCOLOR_ARGB(255, 255, 0, 0), 0);
				}
			}
		}
		if (cheat_state->cheatow.iTracerLine && cheat_state->cheatow.silentaim)
		{
			CVector mySpinePos, TargetSpinePos;
			pPedSelf->GetBonePosition(BONE_PELVIS1, &mySpinePos);
			pPed->GetBonePosition(BONE_PELVIS1, &TargetSpinePos);
			render->DrawLine(CVecToD3DXVEC(mySpinePos), CVecToD3DXVEC(TargetSpinePos), D3DCOLOR_ARGB(255, 255, 0, 0));
		}
		CVector vecBone[55];
		for (int iBone = BONE_PELVIS1; iBone <= BONE_RIGHTFOOT; iBone++)
		{
			switch (iBone)
			{
			case 5: case 4: case 22: case 32: case 23: case 33: case 24: case 34: case 25: case 26: case 35: case 36: case 3: case 2: case 52: case 42: case 53: case 43: case 54: case 44:
				pPed->GetBonePosition((eBone)iBone, &vecBone[iBone]);
				break;
			}
		}

		if (cheat_state->cheatow.iSkeletESP && cheat_state->cheatow.silentaim)
		{
			D3DCOLOR color;
			color = D3DCOLOR_ARGB(255, 255, 0, 0);
			render->DrawLine(CVecToD3DXVEC(vecBone[BONE_NECK]), CVecToD3DXVEC(vecBone[BONE_UPPERTORSO]), color);
			render->DrawLine(CVecToD3DXVEC(vecBone[BONE_UPPERTORSO]), CVecToD3DXVEC(vecBone[BONE_RIGHTSHOULDER]), color);
			render->DrawLine(CVecToD3DXVEC(vecBone[BONE_UPPERTORSO]), CVecToD3DXVEC(vecBone[BONE_LEFTSHOULDER]), color);
			render->DrawLine(CVecToD3DXVEC(vecBone[BONE_LEFTSHOULDER]), CVecToD3DXVEC(vecBone[BONE_LEFTELBOW]), color);
			render->DrawLine(CVecToD3DXVEC(vecBone[BONE_RIGHTSHOULDER]), CVecToD3DXVEC(vecBone[BONE_RIGHTELBOW]), color);
			render->DrawLine(CVecToD3DXVEC(vecBone[BONE_LEFTELBOW]), CVecToD3DXVEC(vecBone[BONE_LEFTWRIST]), color);
			render->DrawLine(CVecToD3DXVEC(vecBone[BONE_RIGHTELBOW]), CVecToD3DXVEC(vecBone[BONE_RIGHTWRIST]), color);
			render->DrawLine(CVecToD3DXVEC(vecBone[BONE_LEFTWRIST]), CVecToD3DXVEC(vecBone[BONE_LEFTHAND]), color);
			render->DrawLine(CVecToD3DXVEC(vecBone[BONE_LEFTHAND]), CVecToD3DXVEC(vecBone[BONE_LEFTTHUMB]), color);
			render->DrawLine(CVecToD3DXVEC(vecBone[BONE_RIGHTWRIST]), CVecToD3DXVEC(vecBone[BONE_RIGHTHAND]), color);
			render->DrawLine(CVecToD3DXVEC(vecBone[BONE_RIGHTHAND]), CVecToD3DXVEC(vecBone[BONE_RIGHTTHUMB]), color);
			render->DrawLine(CVecToD3DXVEC(vecBone[BONE_UPPERTORSO]), CVecToD3DXVEC(vecBone[BONE_SPINE1]), color);
			render->DrawLine(CVecToD3DXVEC(vecBone[BONE_SPINE1]), CVecToD3DXVEC(vecBone[BONE_PELVIS]), color);
			render->DrawLine(CVecToD3DXVEC(vecBone[BONE_PELVIS]), CVecToD3DXVEC(vecBone[BONE_RIGHTKNEE]), color);
			render->DrawLine(CVecToD3DXVEC(vecBone[BONE_PELVIS]), CVecToD3DXVEC(vecBone[BONE_LEFTKNEE]), color);
			render->DrawLine(CVecToD3DXVEC(vecBone[BONE_RIGHTKNEE]), CVecToD3DXVEC(vecBone[BONE_RIGHTANKLE]), color);
			render->DrawLine(CVecToD3DXVEC(vecBone[BONE_LEFTKNEE]), CVecToD3DXVEC(vecBone[BONE_LEFTANKLE]), color);
			render->DrawLine(CVecToD3DXVEC(vecBone[BONE_RIGHTANKLE]), CVecToD3DXVEC(vecBone[BONE_RIGHTFOOT]), color);
			render->DrawLine(CVecToD3DXVEC(vecBone[BONE_LEFTANKLE]), CVecToD3DXVEC(vecBone[BONE_LEFTFOOT]), color);
		}
	}
	return targetid;
}

void NoReload()
{
	if (cheat_state->cheatow.noreload)
	{
		actor_info* negro = g_Players->pLocalPlayer->pSAMP_Actor->pGTA_Ped;
		CPed* Ped = pGameInterface->GetPools()->GetPed((DWORD*)negro);
		Ped->GetPedInterface()->Weapons[Ped->GetPedInterface()->bCurrentWeaponSlot].m_nAmmoInClip = Ped->GetPedInterface()->Weapons[Ped->GetPedInterface()->bCurrentWeaponSlot].m_nAmmoTotal;

	}
}

byte CalculateSpreadOffset(WORD targetID, float* target_pos, float* out_spread)
{
	actor_info* actorInfo = g_Players->pRemotePlayer[targetID]->pPlayerData->pSAMP_Actor->pGTA_Ped;
	if (actorInfo == nullptr) return 255;
	CPed* Ped = pGameInterface->GetPools()->GetPed((DWORD*)actorInfo);
	if (Ped == nullptr) return 255;
	int random_bone;
	byte player_state = g_Players->pRemotePlayer[targetID]->pPlayerData->bytePlayerState;
	if (cheat_state->cheatow.onlyhs) 
	{
		byte bodyIDs[3] = { 8, 7, 6 }; CVector head;
		random_bone = bodyIDs[rand() % 2]; //bodyIDs[rand() % 2]
		Ped->GetTransformedBonePosition((eBone)random_bone, &head);
		out_spread[0] = head.fX - target_pos[0];
		out_spread[1] = head.fY - target_pos[1];
		out_spread[2] = head.fZ - target_pos[2];
		return 9;
	}
	else if (player_state == PLAYER_STATE_ONFOOT)
	{
		CVector rbody; byte bodyIDs[11] = { 1, 2, 4, 5, 6, 8, 21, 31, 41, 51 };
		random_bone = bodyIDs[rand() % 10];
		Ped->GetTransformedBonePosition((eBone)random_bone, &rbody);
		out_spread[0] = rbody.fX - target_pos[0];
		out_spread[1] = rbody.fY - target_pos[1];
		out_spread[2] = rbody.fZ - target_pos[2];
	}
	else if (player_state == PLAYER_STATE_DRIVER || player_state == PLAYER_STATE_PASSENGER)
	{
		byte bodyIDs[3] = { 8, 7, 6 }; CVector head;
		random_bone = bodyIDs[rand() % 2]; //bodyIDs[rand() % 2]
		Ped->GetTransformedBonePosition((eBone)random_bone, &head);
		out_spread[0] = head.fX - target_pos[0];
		out_spread[1] = head.fY - target_pos[1];
		out_spread[2] = head.fZ - target_pos[2];
		return 9;
	}
	if (random_bone == 5 || random_bone == 8 || random_bone == 7 || random_bone == 6) return 9;
	if (random_bone == 4 || random_bone == 21 || random_bone == 3 || random_bone == 31) return 3;
	if (random_bone == 2 || random_bone == 1) return 4;
	if (random_bone == 34 || random_bone == 33 || random_bone == 35 || random_bone == 32 || random_bone == 36) return 5;
	if (random_bone == 24 || random_bone == 23 || random_bone == 25 || random_bone == 22 || random_bone == 26) return 6;
	if (random_bone == 43 || random_bone == 44 || random_bone == 42 || random_bone == 41) return 7;
	if (random_bone == 53 || random_bone == 54 || random_bone == 52 || random_bone == 51) return 8;
	return 255;
}

void RotateQuaternion(float angle_radian, float* quat_w, float* quat_x)
{
	*quat_x = -1 * sinf(angle_radian / 2.0f);
	*quat_w = cosf(angle_radian / 2.0f);
}

void traceLastFunc ( const char *szFunc )
{
	_snprintf_s( g_szLastFunc, sizeof(g_szLastFunc)-1, szFunc );
}

void Log ( const char *fmt, ... )
{
	if ( !g_szWorkingDirectory ) return;

	SYSTEMTIME	time;
	va_list		ap;

	if ( g_flLog == NULL )
	{
		char	filename[512];
		snprintf( filename, sizeof(filename), "%s\\" M0D_FOLDER "%s", g_szWorkingDirectory, "mod_sa.log" );

		g_flLog = fopen( filename, "w" );
		if ( g_flLog == NULL )
			return;
	}

	GetLocalTime( &time );
	fprintf( g_flLog, "[%02d:%02d:%02d.%03d] ", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );
	va_start( ap, fmt );
	vfprintf( g_flLog, fmt, ap );
	va_end( ap );
	fprintf( g_flLog, "\n" );
	fflush( g_flLog );

	if ( g_flLogAll == NULL )
	{
		char	filename_all[512];
		snprintf( filename_all, sizeof(filename_all), "%s\\" M0D_FOLDER "%s", g_szWorkingDirectory, "mod_sa_all.log" );

		g_flLogAll = fopen( filename_all, "a" );
		if ( g_flLogAll == NULL )
			return;
	}

	GetLocalTime( &time );
	fprintf( g_flLogAll, "[%02d-%02d-%02d || %02d:%02d:%02d.%03d] ", time.wDay, time.wMonth, time.wYear, time.wHour,
			 time.wMinute, time.wSecond, time.wMilliseconds );
	va_start( ap, fmt );
	vfprintf( g_flLogAll, fmt, ap );
	va_end( ap );
	fprintf( g_flLogAll, "\n" );
	fflush( g_flLogAll );
}

void LogChatbox ( bool bLast, const char *fmt, ... )
{
	SYSTEMTIME	time;
	va_list		ap;

	if ( g_flLogChatbox == NULL )
	{
		char	filename[512];
		snprintf( filename, sizeof(filename), "%s\\" M0D_FOLDER "%s", g_szWorkingDirectory, "mod_sa_chatbox.log" );

		g_flLogChatbox = fopen( filename, "w" );
		if ( g_flLogChatbox == NULL )
			return;
	}

	GetLocalTime( &time );
	fprintf( g_flLogChatbox, "[%02d:%02d:%02d.%02d] ", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );
	va_start( ap, fmt );
	vfprintf( g_flLogChatbox, fmt, ap );
	va_end( ap );
	fprintf( g_flLogChatbox, "\n" );
	if ( bLast )
		fprintf( g_flLogChatbox, "\n" );
	fflush( g_flLogChatbox );

	if ( g_flLogChatboxAll == NULL )
	{
		char	filename_all[512];
		snprintf( filename_all, sizeof(filename_all), "%s\\" M0D_FOLDER "%s", g_szWorkingDirectory, "mod_sa_chatbox_all.log" );

		g_flLogChatboxAll = fopen( filename_all, "a" );
		if ( g_flLogChatboxAll == NULL )
			return;
	}

	GetLocalTime( &time );
	fprintf( g_flLogChatboxAll, "[%02d-%02d-%02d || %02d:%02d:%02d.%02d] ", time.wDay, time.wMonth, time.wYear,
			 time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );
	va_start( ap, fmt );
	vfprintf( g_flLogChatboxAll, fmt, ap );
	va_end( ap );
	fprintf( g_flLogChatboxAll, "\n" );
	if ( bLast )
		fprintf( g_flLogChatboxAll, "\n" );
	fflush( g_flLogChatboxAll );
}

void setDebugPointer ( void *ptr )
{
	struct debug_info	*debug = &cheat_state->debug;

	if ( debug->hist_pos < DEBUG_HIST_LEN )
	{
		if ( debug->ptr[debug->hist_pos] != NULL && ptr != NULL )
			debug->hist_pos++;
		else if ( debug->hist_pos == 1 && ptr == NULL )
			debug->hist_pos++;
	}
	else
	{
		memmove( debug->ptr, debug->ptr + 1, sizeof(debug->ptr) - sizeof(debug->ptr[0]) );
		memmove( debug->offset, debug->offset + 1, sizeof(debug->offset) - sizeof(debug->offset[0]) );
	}

	debug->ptr[debug->hist_pos] = (uint8_t *)ptr;
	debug->offset[debug->hist_pos] = 0;
	debug->data_prev_clear = 1;
}

static int init ( void )
{
	traceLastFunc( "init()" );

		if ( GetModuleFileName(g_hDllModule, g_szWorkingDirectory, sizeof(g_szWorkingDirectory) - 32) != 0 )
		{
			if ( strrchr(g_szWorkingDirectory, '\\') != NULL )
				*strrchr( g_szWorkingDirectory, '\\' ) = 0;
			else
				strcpy( g_szWorkingDirectory, "." );
		}
		else
		{
			strcpy( g_szWorkingDirectory, "." );
		}

		// Hello World
		Log( "Initializing %s", NAME );
		Log( "Compiled: %s CL:%d", COMPILE_DT, COMPILE_VERSION );

		// log windows version for people that forget to report it
		WindowsInfo.osPlatform = (int) * (DWORD *)GTAvar_osPlatform;
		WindowsInfo.osVer = (int) * (DWORD *)GTAvar_osVer;
		WindowsInfo.winVer = (int) * (DWORD *)GTAvar_winVer;
		WindowsInfo.winMajor = (int) * (DWORD *)GTAvar_winMajor;
		if ( WindowsInfo.osPlatform == 2 )
			Log( "OS: Windows Version %d.%d.%d", WindowsInfo.winMajor, WindowsInfo.winVer, WindowsInfo.osVer );
		else
			Log( "OS: code %d (%d.%d.%d)", WindowsInfo.osPlatform, WindowsInfo.winMajor, WindowsInfo.winVer, WindowsInfo.osVer );

		/*
		int D3D9UseVersion = (int)*(DWORD*)GTAvar__D3D9UseVersion;
		Log("D3D9UseVersion: %d", D3D9UseVersion);
		int DXVersion = (int)*(DWORD*)GTAvar__DXVersion;
		Log("DXVersion: %d", DXVersion);
		int windowsVersion = (int)*(DWORD*)GTAvar__windowsVersion;
		Log("windowsVersion: %d", windowsVersion);
		int CPUSupportsMMX = (int)*(DWORD*)__rwD3D9CPUSupportsMMX;
		int CPUSupports3DNow = (int)*(DWORD*)__rwD3D9CPUSupports3DNow;
		int CPUSupportsSSE = (int)*(DWORD*)__rwD3D9CPUSupportsSSE;
		int CPUSupportsSSE2 = (int)*(DWORD*)__rwD3D9CPUSupportsSSE2;
		if (!CPUSupportsMMX)
			Log("CPU Supports MMX: %d", CPUSupportsMMX);
		if (!CPUSupports3DNow)
			Log("CPU Supports 3DNow: %d", CPUSupports3DNow);
		if (!CPUSupportsSSE)
			Log("CPU Supports SSE: %d", CPUSupportsSSE);
		if (!CPUSupportsSSE2)
			Log("CPU Supports SSE2: %d", CPUSupportsSSE2);
		*/
#pragma warning( disable : 4127 )
		if ( sizeof(struct vehicle_info) != 2584 )
		{
			Log( "sizeof(struct vehicle_info) == %d, aborting.", sizeof(struct vehicle_info) );
			return 0;
		}

		if ( sizeof(struct actor_info) != 1988 )
		{
			Log( "sizeof(struct actor_info) == %d, aborting.", sizeof(struct actor_info) );
			return 0;
		}
#pragma warning( default : 4127 )

		ini_load();
		
		if ( !set.i_have_edited_the_ini_file )
		{
			MessageBox( 0, "Looks like you've not edited the .ini file like you were told to!\n""\n"
				"Before you can use mod_sa, you have to set \"i_have_edited_the_ini_file\" to true.\n"
				"We did this so you would read the INI file to see the configurability of mod_sa.\n",
				"You're a retard.", 0 );
			ShellExecute( 0, "open", "notepad", M0D_FOLDER INI_FILE, g_szWorkingDirectory, SW_SHOW );
		}

		// get SAMP and set g_dwSAMP_Addr
		getSamp();

	return 1;
}

LONG WINAPI unhandledExceptionFilter ( struct _EXCEPTION_POINTERS *ExceptionInfo )
{
	Log( " ---------------------------------------------------------------------" );
	Log( " %s has crashed.", NAME );
	Log( " Base address: 0x%p, SA:MP base address: 0x%p", g_hDllModule, g_dwSAMP_Addr );
	Log( " Exception at address: 0x%p, Last function processed: %s", ExceptionInfo->ExceptionRecord->ExceptionAddress, g_szLastFunc );

	int m_ExceptionCode = ExceptionInfo->ExceptionRecord->ExceptionCode;
	int m_exceptionInfo_0 = ExceptionInfo->ExceptionRecord->ExceptionInformation[0];
	int m_exceptionInfo_1 = ExceptionInfo->ExceptionRecord->ExceptionInformation[1];
	int m_exceptionInfo_2 = ExceptionInfo->ExceptionRecord->ExceptionInformation[2];
	switch ( m_ExceptionCode )
	{
	case EXCEPTION_ACCESS_VIOLATION:
		Log( " Cause: EXCEPTION_ACCESS_VIOLATION" );
		if ( m_exceptionInfo_0 == 0 )
		{
			// bad read
			Log( " Attempted to read from: 0x%08x", m_exceptionInfo_1 );
		}
		else if ( m_exceptionInfo_0 == 1 )
		{
			// bad write
			Log( " Attempted to write to: 0x%08x", m_exceptionInfo_1 );
		}
		else if ( m_exceptionInfo_0 == 8 )
		{
			// user-mode data execution prevention (DEP)
			Log( " Data Execution Prevention (DEP) at: 0x%08x", m_exceptionInfo_1 );
		}
		else
		{
			// unknown, shouldn't happen
			Log( " Unknown access violation at: 0x%08x", m_exceptionInfo_1 );
		}
		break;

	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		Log( " Cause: EXCEPTION_ARRAY_BOUNDS_EXCEEDED" );
		break;

	case EXCEPTION_BREAKPOINT:
		Log( " Cause: EXCEPTION_BREAKPOINT" );
		break;

	case EXCEPTION_DATATYPE_MISALIGNMENT:
		Log( " Cause: EXCEPTION_DATATYPE_MISALIGNMENT" );
		break;

	case EXCEPTION_FLT_DENORMAL_OPERAND:
		Log( " Cause: EXCEPTION_FLT_DENORMAL_OPERAND" );
		break;

	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		Log( " Cause: EXCEPTION_FLT_DIVIDE_BY_ZERO" );
		break;

	case EXCEPTION_FLT_INEXACT_RESULT:
		Log( " Cause: EXCEPTION_FLT_INEXACT_RESULT" );
		break;

	case EXCEPTION_FLT_INVALID_OPERATION:
		Log( " Cause: EXCEPTION_FLT_INVALID_OPERATION" );
		break;

	case EXCEPTION_FLT_OVERFLOW:
		Log( " Cause: EXCEPTION_FLT_OVERFLOW" );
		break;

	case EXCEPTION_FLT_STACK_CHECK:
		Log( " Cause: EXCEPTION_FLT_STACK_CHECK" );
		break;

	case EXCEPTION_FLT_UNDERFLOW:
		Log( " Cause: EXCEPTION_FLT_UNDERFLOW" );
		break;

	case EXCEPTION_ILLEGAL_INSTRUCTION:
		Log( " Cause: EXCEPTION_ILLEGAL_INSTRUCTION" );
		break;

	case EXCEPTION_IN_PAGE_ERROR:
		Log( " Cause: EXCEPTION_IN_PAGE_ERROR" );
		if ( m_exceptionInfo_0 == 0 )
		{
			// bad read
			Log( " Attempted to read from: 0x%08x", m_exceptionInfo_1 );
		}
		else if ( m_exceptionInfo_0 == 1 )
		{
			// bad write
			Log( " Attempted to write to: 0x%08x", m_exceptionInfo_1 );
		}
		else if ( m_exceptionInfo_0 == 8 )
		{
			// user-mode data execution prevention (DEP)
			Log( " Data Execution Prevention (DEP) at: 0x%08x", m_exceptionInfo_1 );
		}
		else
		{
			// unknown, shouldn't happen
			Log( " Unknown access violation at: 0x%08x", m_exceptionInfo_1 );
		}

		// log NTSTATUS
		Log( " NTSTATUS: 0x%08x", m_exceptionInfo_2 );

		/*
		NT_SUCCESS(Status)
			Evaluates to TRUE if the return value specified by Status is a success type (0 - 0x3FFFFFFF) or an informational type (0x40000000 - 0x7FFFFFFF).
		NT_INFORMATION(Status)
			Evaluates to TRUE if the return value specified by Status is an informational type (0x40000000 - 0x7FFFFFFF).
		NT_WARNING(Status)
			Evaluates to TRUE if the return value specified by Status is a warning type (0x80000000 - 0xBFFFFFFF).
		NT_ERROR(Status)
			Evaluates to TRUE if the return value specified by Status is an error type (0xC0000000 - 0xFFFFFFFF)
		*/
		break;

	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		Log( " Cause: EXCEPTION_INT_DIVIDE_BY_ZERO" );
		break;

	case EXCEPTION_INT_OVERFLOW:
		Log( " Cause: EXCEPTION_INT_OVERFLOW" );
		break;

	case EXCEPTION_INVALID_DISPOSITION:
		Log( " Cause: EXCEPTION_INVALID_DISPOSITION" );
		break;

	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		Log( " Cause: EXCEPTION_NONCONTINUABLE_EXCEPTION" );
		break;

	case EXCEPTION_PRIV_INSTRUCTION:
		Log( " Cause: EXCEPTION_PRIV_INSTRUCTION" );
		break;

	case EXCEPTION_SINGLE_STEP:
		Log( " Cause: EXCEPTION_SINGLE_STEP" );
		break;

	case EXCEPTION_STACK_OVERFLOW:
		Log( " Cause: EXCEPTION_STACK_OVERFLOW" );
		break;

	case DBG_CONTROL_C:
		Log( " Cause: DBG_CONTROL_C (WTF!)" );
		break;

	default:
		Log( " Cause: %08x", m_ExceptionCode );
	}

	Log( " EAX: 0x%08x || ESI: 0x%08x", ExceptionInfo->ContextRecord->Eax, ExceptionInfo->ContextRecord->Esi );
	Log( " EBX: 0x%08x || EDI: 0x%08x", ExceptionInfo->ContextRecord->Ebx, ExceptionInfo->ContextRecord->Edi );
	Log( " ECX: 0x%08x || EBP: 0x%08x", ExceptionInfo->ContextRecord->Ecx, ExceptionInfo->ContextRecord->Ebp );
	Log( " EDX: 0x%08x || ESP: 0x%08x", ExceptionInfo->ContextRecord->Edx, ExceptionInfo->ContextRecord->Esp );

	Log( " ---------------------------------------------------------------------" );

	if (OrigExceptionFilter)
		return OrigExceptionFilter(ExceptionInfo);

	return EXCEPTION_CONTINUE_SEARCH;
}

void EnableWindowsAero()
{
	typedef HRESULT (WINAPI *DwmEnableComposition_t)(UINT uCompositionAction);
	HMODULE dwmapi = LoadLibrary("dwmapi.dll");
	if (dwmapi == NULL)
		return;

	DwmEnableComposition_t dwmEnableComposition = (DwmEnableComposition_t)GetProcAddress(dwmapi, "DwmEnableComposition");
	if (dwmEnableComposition != NULL)
		dwmEnableComposition(1);

	FreeLibrary(dwmapi);
}

void __stdcall mainloop()
{
	// get actual d3d9.dll and proxy original D3D9Device
	g_hOrigDll = GetModuleHandle("d3d9.dll");
	if (g_hOrigDll == NULL || g_hOrigDll == INVALID_HANDLE_VALUE)
		return;

	static bool initialize = false;
	if (initialize)
		return;
	initialize = true;

	if (init()) {
		stringCompressor->AddReference();
		pDirect3D9 = new proxyIDirect3D9(*(IDirect3D9 **)0xC97C20);
		*(IDirect3D9 **)0xC97C20 = pDirect3D9;
		pPresentParam = *(D3DPRESENT_PARAMETERS*)0xC9C040;
		*(IDirect3DDevice9 **)0xC97C28 = new proxyIDirect3DDevice9(*(IDirect3DDevice9 **)0xC97C28);
		// Don't pause when in ESC menu
		memset((BYTE*)0x74542B, 0x90, 8);
		// Keep SAMP working when not in focus
		memset((BYTE*)0x53EA88, 0x90, 6);

		// Disable menu after alt-tab
		memset((BYTE*)0x53BC78, 0x00, 1);

		// ALLOW ALT+TABBING WITHOUT PAUSING
		memset((BYTE*)0x748A8D, 0x90, 6);
		*(int*)0x555854 = -1869574000;
		*(BYTE*)0x555858 = 144;
		*(BYTE*)0x53E159 = -61;  //IDK
		*(int*)0x53C1AB = -1869574000; //INT
		*(BYTE*)0x53C1AF = -112; //IDK
		*(uint16_t*)0x5E6789 = -5744; //INT16
		*(int*)0x70BDAB = -1869574000; // INT
		*(uint16_t*)0x70BDAF = -28528; //INT 16
		*(int*)0x70C75A = -1869574000; //INT
		*(uint16_t*)0x70C75E = -28528; //INT 16
		DWORD oldProt12563;
		VirtualProtect(reinterpret_cast<LPVOID>(0x0053E0BE), 15, PAGE_READWRITE, &oldProt12563);
		memset(reinterpret_cast<void*>(0x0053E0BE), 0x90, 15);
		VirtualProtect(reinterpret_cast<LPVOID>(0x0053E0BE), 15, oldProt12563, &oldProt12563);
		((void(__cdecl*)())0x6FC5A0)();
	}
	cheat_patches_installRuntimePatches();
}

BOOL APIENTRY DllMain ( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved )
{
	static CCallHook *mhook;
	switch ( ul_reason_for_call )
	{
	case DLL_PROCESS_ATTACH:
		mhook = new CCallHook(reinterpret_cast<void*>(0x00748DA3), 6);
		mhook->enable(mainloop);
		DisableThreadLibraryCalls( hModule );
		// windows aero fix for windows 7 and vista
		EnableWindowsAero();
		g_hDllModule = hModule;
		OrigExceptionFilter = SetUnhandledExceptionFilter( unhandledExceptionFilter );
		break;

	case DLL_PROCESS_DETACH:
		if ( g_hOrigDll != NULL )
		{
			stringCompressor->RemoveReference();
			menu_free_all();
			ini_free();
			Log( "Exited\n" );
			if ( set.chatbox_logging && g_SAMP )
				LogChatbox( true, "Logging ended" );
		}

		if ( g_flLog != NULL )
		{
			fclose( g_flLog );
			g_flLog = NULL;
		}

		if ( g_flLogAll != NULL )
		{
			fclose( g_flLogAll );
			g_flLogAll = NULL;
		}

		if ( set.chatbox_logging )
		{
			if ( g_flLogChatbox != NULL )
			{
				fclose( g_flLogChatbox );
				g_flLogChatbox = NULL;
			}

			if ( g_flLogChatboxAll != NULL )
			{
				fclose( g_flLogChatboxAll );
				g_flLogChatboxAll = NULL;
			}
		}

		if (set.ant_tweak_menu)
			TwTerminate();

		delete mhook;
			
		break;
	}

	return true;
}
