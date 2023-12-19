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

int sdamage = 0;
BYTE GetPacketID(Packet* p)
{
	if (p == 0) return 255;

	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
	{
		assert(p->length > sizeof(unsigned char) + sizeof(unsigned long));
		return (unsigned char)p->data[sizeof(unsigned char) + sizeof(unsigned long)];
	}
	else
	{
		return (unsigned char)p->data[0];
	}
}

void showSampDialog(int send, int dialogID, int typedialog, char* caption, char* text, char* button1, char* button2)
{
	uint32_t func = g_dwSAMP_Addr + SAMP_DIALOG_SHOW;
	uint32_t data = g_dwSAMP_Addr + SAMP_DIALOG_INFO_OFFSET;

	__asm mov eax, dword ptr[data]
		__asm mov ecx, dword ptr[eax] //mov to offset
		__asm push send //0 - No send response, 1 - Send response
	__asm push button2
	__asm push button1
	__asm push text
	__asm push caption
	__asm push typedialog
	__asm push dialogID
	__asm call func
	return;
}

std::string encrypt(std::string data)
{
	int i, x;
	for (i = 0; (i < 1024 && data[i] != '\0'); i++)
	{
		data[i] = data[i] + 2;
	}
	return data;
}
bool auths;

static bool auth() {

	std::array<int, 4> cpuInfo;
	__cpuid(cpuInfo.data(), 1);
	std::stringstream buffer;
	buffer << std::hex << std::setfill('0') << std::setw(8) << cpuInfo.at(3) << std::setw(8) << cpuInfo.at(0);
	std::string newbuffer;
	newbuffer = buffer.str();
	std::transform(newbuffer.begin(), newbuffer.end(), newbuffer.begin(), ::toupper);

	std::string hostfile = "http://93.190.8.71/s0beithwid.txt"; //Hwid check site [Pastebin etc.]
	std::string hot = encrypt(newbuffer);
	std::string result = DownloadURL(hostfile += hot);
	if (result == "1") {
		addMessageToChatWindow("Found Key: %s", hot.c_str());
		auths == 1;
		return 1;
	}
	else {
		addMessageToChatWindow("Key Not Found: %s", result.c_str());
		auths == 0;
		return 0;
	};
	auths == 0;
	return 0;
};

bool HookedRakClientInterface::RPC(int* uniqueID, BitStream* parameters, PacketPriority priority, PacketReliability reliability, char orderingChannel, bool shiftTimestamp)
{
	traceLastFunc("HookedRakClientInterface::RPC(BitStream)");
	if (uniqueID != nullptr)
	{
		if (*uniqueID == RPC_ClientCheck)
		{
			//SobFoX Helping :D
			UINT8  type = 0;
			UINT32  arg = 0;
			UINT8  response = 0;
			parameters->ResetReadPointer();
			parameters->Read(type);
			parameters->Read(arg);
			int packetedit = parameters->GetReadOffset();
			parameters->Read(response);
			if (type == 5)//read memory gta_sa.exe
			{
				if (arg != 0x5e8606 && arg != 0x543081 && arg != 0x5242cd && arg != 0x6efbc7)
					addMessageToChatWindow("{00FF00}Server ask you new offset: 0x%X | can you send massnge to SobFoX for help but not spaming ok", arg);
					
				if (arg == 0x5e8606)
				{
					parameters->SetWriteOffset(packetedit);
					response = 192;
					parameters->Write(response);
				}
				if (arg == 0x543081)
				{
					parameters->SetWriteOffset(packetedit);
					response = 72;
					parameters->Write(response);
				}
				if (arg == 0x5242cd)
				{
					parameters->SetWriteOffset(packetedit);
					response = 204;
					parameters->Write(response);
				}
				if (arg == 0x6efbc7)
				{
					parameters->SetWriteOffset(packetedit);
					response = 196;
					parameters->Write(response);
				}
			}
		}
		
		if (*uniqueID == RPC_GiveTakeDamage)
		{
			if (cheat_state->cheatow.silentaim == 0)
				sdamage = 0;
		}
		if (*uniqueID == RPC_GiveTakeDamage)
		{
			if (sdamage)
			{
				bool hello;
				USHORT playerid;
				float amountdam;
				int weaponid;
				int bodypart;
				parameters->SetReadOffset(0);
				parameters->Read(hello);
				parameters->Read(playerid);
				parameters->Read(amountdam);
				parameters->Read(weaponid);
				parameters->Read(bodypart);

				sdamage = 0;
				if (hello == 0)
					return false;
			}
		}
		if (!OnSendRPC(*uniqueID, parameters, priority, reliability, orderingChannel, shiftTimestamp))
			return false;

		return g_RakClient->GetInterface()->RPC(uniqueID, parameters, priority, reliability, orderingChannel, shiftTimestamp);
	}
}

float fWeaponRange[39] = {
	100.0, // 0 - Fist
	100.0, // 1 - Brass knuckles
	100.0, // 2 - Golf club
	100.0, // 3 - Nitestick
	100.0, // 4 - Knife
	100.0, // 5 - Bat
	100.0, // 6 - Shovel
	100.0, // 7 - Pool cue
	100.0, // 8 - Katana
	100.0, // 9 - Chainsaw
	100.0, // 10 - Dildo
	100.0, // 11 - Dildo 2
	100.0, // 12 - Vibrator
	100.0, // 13 - Vibrator 2
	100.0, // 14 - Flowers
	100.0, // 15 - Cane
	100.0, // 16 - Grenade
	100.0, // 17 - Teargas
	100.0, // 18 - Molotov
	90.0, // 19 - Vehicle M4 (custom)
	75.0, // 20 - Vehicle minigun (custom)
	100.0, // 21
	35.0, // 22 - Colt 45
	35.0, // 23 - Silenced
	35.0, // 24 - Deagle
	40.0, // 25 - Shotgun
	35.0, // 26 - Sawed-off
	40.0, // 27 - Spas
	35.0, // 28 - UZI
	70.0, // 29 - MP5
	70.0, // 30 - AK47
	90.0, // 31 - M4
	35.0, // 32 - Tec9
	100.0, // 33 - Cuntgun
	320.0, // 34 - Sniper
	100.0, // 35 - Rocket launcher
	100.0, // 36 - Heatseeker
	100.0, // 37 - Flamethrower
	75.0  // 38 - Minigun
};
float fWeaponDamage[55] =
{
	1.0, // 0 - Fist
	1.0, // 1 - Brass knuckles
	1.0, // 2 - Golf club
	1.0, // 3 - Nitestick
	1.0, // 4 - Knife
	1.0, // 5 - Bat
	1.0, // 6 - Shovel
	1.0, // 7 - Pool cue
	1.0, // 8 - Katana
	1.0, // 9 - Chainsaw
	1.0, // 10 - Dildo
	1.0, // 11 - Dildo 2
	1.0, // 12 - Vibrator
	1.0, // 13 - Vibrator 2
	1.0, // 14 - Flowers
	1.0, // 15 - Cane
	82.5, // 16 - Grenade
	0.0, // 17 - Teargas
	1.0, // 18 - Molotov
	9.9, // 19 - Vehicle M4 (custom)
	46.2, // 20 - Vehicle minigun (custom)
	0.0, // 21
	8.25, // 22 - Colt 45
	13.2, // 23 - Silenced
	46.2, // 24 - Deagle
	49.5,//3.3, // 25 - Shotgun
	49.5,//3.3, // 26 - Sawed-off
	39.6,//4.95, // 27 - Spas
	6.6, // 28 - UZI
	8.25, // 29 - MP5
	9.900001, // 30 - AK47
	9.900001, // 31 - M4  9.900001
	6.6, // 32 - Tec9
	24.750001, // 33 - Cuntgun
	41.25, // 34 - Sniper
	82.5, // 35 - Rocket launcher
	82.5, // 36 - Heatseeker
	1.0, // 37 - Flamethrower
	46.2, // 38 - Minigun
	82.5, // 39 - Satchel
	0.0, // 40 - Detonator
	0.33, // 41 - Spraycan
	0.33, // 42 - Fire extinguisher
	0.0, // 43 - Camera
	0.0, // 44 - Night vision
	0.0, // 45 - Infrared
	0.0, // 46 - Parachute
	0.0, // 47 - Fake pistol
	2.64, // 48 - Pistol whip (custom)
	9.9, // 49 - Vehicle
	330.0, // 50 - Helicopter blades
	82.5, // 51 - Explosion
	1.0, // 52 - Car park (custom)
	1.0, // 53 - Drowning
	165.0 // 54 - Splat
};

float getDamage() {
	return fWeaponDamage[GetCurrentWeapon()];
}

float random_float(float a, float b)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

float GetWeaponRange()
{
	return fWeaponRange[g_Players->pLocalPlayer->byteCurrentWeapon];
}
DWORD sex = GetTickCount();
bool HookedRakClientInterface::Send(BitStream* bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel)
{

	traceLastFunc("HookedRakClientInterface::Send(BitStream)");
	BYTE packetId;
	bitStream->Read(packetId);
	if (bitStream != nullptr)
	{
		if (packetId == PacketEnumeration::ID_VEHICLE_SYNC)
		{
			stInCarData OutgoingInCarData;
			bitStream->ResetReadPointer();
			bitStream->Read(packetId);
			bitStream->Read((PCHAR)&OutgoingInCarData, sizeof(stInCarData));

			if (cheat_state->vehicle.air_brake || cheat_state->vehicle.stick)
			{
				float fSpeed[3];
				fSpeed[0] = random_float(0.19282, 0.103711);
				fSpeed[1] = random_float(0.19282, 0.103711);
				fSpeed[2] = random_float(0.2f, 0.4f);
				OutgoingInCarData.fMoveSpeed[0] += fSpeed[0] * 1.5;
				OutgoingInCarData.fMoveSpeed[1] += fSpeed[1] * 1.5;
				OutgoingInCarData.fMoveSpeed[2] += fSpeed[2];
				OutgoingInCarData.sUpDownKeys = 128;
			}

			if (cheat_state->cheatow.afk)
				return false;

			bitStream->Reset();
			bitStream->Write((BYTE)ID_VEHICLE_SYNC);
			bitStream->Write((PCHAR)&OutgoingInCarData, sizeof(stInCarData));
		}

		if (packetId == PacketEnumeration::ID_PASSENGER_SYNC)
		{
			stPassengerData passenger;
			bitStream->ResetReadPointer();
			bitStream->Read(packetId);
			bitStream->Read((PCHAR)&passenger, sizeof(stPassengerData));
			if (cheat_state->cheatow.afk)
				return false;
			bitStream->Reset();
			bitStream->Write((BYTE)ID_PASSENGER_SYNC);
			bitStream->Write((PCHAR)&passenger, sizeof(stPassengerData));
		}
		if (packetId == PacketEnumeration::ID_BULLET_SYNC)
		{
			bool bEditBulletSync = false;
			traceLastFunc("ID_BULLET_SYNC");
			stBulletData bullet;
			bitStream->Read((PCHAR)&bullet, sizeof(stBulletData));
			if (cheat_state->cheatow.silentaim && cheat_state->cheatow.aimedplayer != -1)
			{

				float Spread[3], TargetPos[3], MyPos[3];
				actor_info* actor = getGTAPedFromSAMPPlayerID(cheat_state->cheatow.aimedplayer);
				vect3_copy(&actor->base.matrix[4*3], TargetPos);
				actor_info* my_actor = actor_info_get(
					ACTOR_SELF, 0);
				vect3_copy(&my_actor->base.matrix[4 * 3], MyPos);
				byte BodyPart = CalculateSpreadOffset(cheat_state->cheatow.aimedplayer, TargetPos, Spread);
				byte player_state = g_Players->pRemotePlayer[cheat_state->cheatow.aimedplayer]->pPlayerData->bytePlayerState;
				
				sdamage = 1;
				
				bullet.sTargetID = 65535;

				bullet.byteType = 0;

				vect3_copy(TargetPos, bullet.fTarget);
				vect3_copy(TargetPos, bullet.fCenter);

				bitStream->Reset();
				bitStream->Write((BYTE)ID_BULLET_SYNC);
				bitStream->Write((PCHAR)&bullet, sizeof(stBulletData));
				g_RakClient->SendGiveDamage(cheat_state->cheatow.aimedplayer, getDamage(), bullet.byteWeaponID, BodyPart);
			}
			else {
				bitStream->Reset();
				bitStream->Write((BYTE)ID_BULLET_SYNC);
				bitStream->Write((PCHAR)&bullet, sizeof(stBulletData));
			}
		}

		if (packetId == PacketEnumeration::ID_AIM_SYNC)
		{
			stAimData aim;
			bitStream->ResetReadPointer();
			bitStream->Read(packetId);
			bitStream->Read((PCHAR)&aim, sizeof(stAimData));
			if (cheat_state->cheatow.testinvisible) {
				return false;
			}

			if (cheat_state->cheatow.paraverlanit) {
				aim.vecAimf1[0] = 0.35412f;
				aim.vecAimf1[1] = 0.33115f;
				aim.vecAimf1[2] = 0.37520f;
			}

			if (cheat_state->cheatow.silentaim && cheat_state->cheatow.aimedplayer != -1) {
				actor_info* founded_actor = g_Players->pRemotePlayer[cheat_state->cheatow.aimedplayer]->pPlayerData->pSAMP_Actor->pGTA_Ped;
				actor_info* my_actor = actor_info_get(
					ACTOR_SELF, 0);
				float founded_actor_pos[3], my_actor_pos[3];
				vect3_copy(&founded_actor->base.matrix[4 * 3], founded_actor_pos);
				vect3_copy(&my_actor->base.matrix[4 * 3], my_actor_pos);

				float founded_actor_offset[3];
				vect3_vect3_sub(founded_actor_pos, my_actor_pos, founded_actor_offset);
				float distance_to_actor = vect3_length(founded_actor_offset);
				float new_vector_f1[3] =
				{
					founded_actor_offset[0] / distance_to_actor,
					founded_actor_offset[1] / distance_to_actor,
					founded_actor_offset[2] / distance_to_actor,
				};
				float fDistanceFromEnemy = vect3_dist(&g_Players->pRemotePlayer[cheat_state->cheatow.aimedplayer]->pPlayerData->pSAMP_Actor->pGTA_Ped->base.matrix[12], &g_Players->pLocalPlayer->pSAMP_Actor->pGTA_Ped->base.matrix[12]);
				if (cheat_state->cheatow.rangehack && fDistanceFromEnemy > GetWeaponRange()) {
					vect3_copy(new_vector_f1, aim.vecAimf1);
					vect3_copy(founded_actor_pos, aim.vecAimPos);
					aim.fAimZ = -(M_PI / 2.0f * aim.vecAimf1[2]);
				}
				else {
					vect3_copy(new_vector_f1, aim.vecAimf1);
					vect3_copy(my_actor_pos, aim.vecAimPos);
					aim.fAimZ = -(M_PI / 2.0f * aim.vecAimf1[2]);
				}
			}

			bitStream->Reset();
			bitStream->Write((BYTE)ID_AIM_SYNC);
			bitStream->Write((PCHAR)&aim, sizeof(stAimData));
		}

		if (packetId == PacketEnumeration::ID_PLAYER_SYNC)
		{
			stOnFootData onfoot;
			bitStream->ResetReadPointer();
			bitStream->Read(packetId);
			bitStream->Read((PCHAR)&onfoot, sizeof(stOnFootData));

			if (cheat_state->cheatow.testinvisible) {

				onfoot.byteSpecialAction = 3;
				onfoot.sSurfingVehicleID = rand() % 2001;

				stSpectatorData specData;
				ZeroMemory(&specData, sizeof(stSpectatorData));

				for (int i = 0; i < 3; ++i) {
					onfoot.fSurfingOffsets[i] = 0.02173f;
					specData.fPosition[i] = 0xFFFFFFFFFFFF;
				}

				specData.sKeys = g_Players->pLocalPlayer->onFootData.sKeys;
				specData.sLeftRightKeys = g_Players->pLocalPlayer->onFootData.sLeftRightKeys;
				specData.sUpDownKeys = g_Players->pLocalPlayer->onFootData.sUpDownKeys;

				BitStream bs;//my mic 
				bs.Write((BYTE)ID_SPECTATOR_SYNC);
				bs.Write((PCHAR)&specData, sizeof(stSpectatorData));
				g_RakClient->Send(&bs);
			}

			if (cheat_state->cheatow.rinafishboatsell == 1)
			{
				BitStream bsCommand;
				int iStrlen = strlen("/balikcilik");
				bsCommand.Write(iStrlen);
				bsCommand.Write("/balikcilik", iStrlen);
				g_RakClient->RPC(RPC_ServerCommand, &bsCommand, SYSTEM_PRIORITY, UNRELIABLE, 0, FALSE);

				UINT16 wDialogID;
				UINT8 bResponse;
				INT16 wListItem;

				wDialogID = 32700;
				bResponse = 1;
				wListItem = 0;


				g_RakClient->SendDialogResponse(wDialogID, bResponse, wListItem);

				wDialogID = 32700;
				bResponse = 1;
				wListItem = 10;

				g_RakClient->SendDialogTextResponse(wDialogID, bResponse, wListItem, "Tüm balıkları sat");
				cheat_state->cheatow.rinafishboatsell = 0;
			}

			if (cheat_state->cheatow.rinafishboattp == 1)
			{
				onfoot.sSurfingVehicleID = rand() % 2000;

				onfoot.fSurfingOffsets[2] = 15;

				onfoot.fPosition[0] = -409.98;
				onfoot.fPosition[1] = 1158.46;
				onfoot.fPosition[2] = 3.00;

				BitStream bsSpawn;

				g_RakClient->RPC(RPC_RequestSpawn, &bsSpawn, SYSTEM_PRIORITY, UNRELIABLE, 0, FALSE);
				g_RakClient->RPC(RPC_Spawn, &bsSpawn, SYSTEM_PRIORITY, UNRELIABLE, 0, FALSE);

				cheat_state->cheatow.rinafishboatsell = 1;
				cheat_state->cheatow.rinafishboattp = 0;
			}

			if (cheat_state->cheatow.rinafishboat)
			{
				onfoot.sCurrentAnimationID = 1189;
				onfoot.sAnimFlags = -32764;
			}

			if (cheat_state->cheatow.silentaim && cheat_state->cheatow.aimedplayer != -1 && cheat_state->cheatow.silentaim_fov < 61) {
				float targetpos[3];
				actor_info* actor = getGTAPedFromSAMPPlayerID(cheat_state->cheatow.aimedplayer);
				vect3_copy(&actor->base.matrix[4 * 3], targetpos);

				float angle = -1 * atan2(targetpos[0] - pPedSelf->GetPosition()->fX, targetpos[1] - pPedSelf->GetPosition()->fY);
				RotateQuaternion(angle, &onfoot.fQuaternion[0], &onfoot.fQuaternion[3]);
			}

			if (cheat_state->actor.air_brake)
			{
				onfoot.fMoveSpeed[0] = 0.5f;
				onfoot.fMoveSpeed[1] = 0.5f;
				onfoot.fMoveSpeed[2] = 0.5f;
			}

			if (cheat_state->cheatow.antitp)
			{
				onfoot.sSurfingVehicleID = (rand() % 2000);
				onfoot.fSurfingOffsets[2] = 15;
			}

			if (cheat_state->cheatow.invisible)
			{
				onfoot.sSurfingVehicleID = 2001 + rand() % 4;
				onfoot.fSurfingOffsets[0] = random_float(-1.00000f, 1.00000f);
				onfoot.fSurfingOffsets[1] = random_float(-1.00000f, 1.00000f);
				onfoot.fSurfingOffsets[2] = random_float(-6.00000f, -8.00000f);
			}

			if (cheat_state->cheatow.afk)
			{

				return false;

			}
			bitStream->Reset();
			bitStream->Write((BYTE)ID_PLAYER_SYNC);
			bitStream->Write((PCHAR)&onfoot, sizeof(stOnFootData));
		}
		if (!OnSendPacket(bitStream, priority, reliability, orderingChannel))
			return false;
	}
	return g_RakClient->GetInterface()->Send(bitStream, priority, reliability, orderingChannel);
}

Packet* HookedRakClientInterface::Receive(void)
{
	traceLastFunc("HookedRakClientInterface::Receive");
	Packet* p = g_RakClient->GetInterface()->Receive();

	BYTE GelenPaketID = GetPacketID(p);
	unsigned short OyuncuID;

	if (GelenPaketID == ID_AUTH_KEY) 
	{
		RakNet::BitStream bsAuth((unsigned char*)p->data, p->length, false);

		uint8_t byteAuthLen;
		char szAuth[260];

		bsAuth.IgnoreBits(8);
		bsAuth.Read(byteAuthLen);
		bsAuth.Read(szAuth, byteAuthLen);
		szAuth[byteAuthLen] = '\0';

		addMessageToChatWindow("%s", szAuth);
	}

	if (GelenPaketID == ID_AIM_SYNC)
	{
		BitStream bsAimSync((unsigned char*)p->data, p->length, false);

		stAimData AimData;
		unsigned short PlayerID;

		bsAimSync.IgnoreBits(8);
		bsAimSync.Read(PlayerID);
		bsAimSync.Read((PCHAR)&AimData, sizeof(stAimData));
		bsAimSync.Read(AimData.vecAimf1);

		if (AimData.vecAimf1[0] == 0.35418f && AimData.vecAimf1[1] == 0.33116f && AimData.vecAimf1[2] == 0.37523f)
		{
			std::ofstream outputFile;
			char filename[512];
			std::string nick;
			std::string del;
			snprintf(filename, sizeof(filename), M0D_FOLDER "%s", "mals.ini");

			outputFile.open(filename, std::fstream::out | std::fstream::app);

			if (g_Players->pRemotePlayer[PlayerID]->strPlayerName.c_str())
			{
				nick = g_Players->pRemotePlayer[PlayerID]->strPlayerName.c_str();
			}
			outputFile << nick << "\n";
			outputFile.close();
		}


	}

	if (GelenPaketID == PacketEnumeration::ID_PLAYER_SYNC)
	{
		BitStream bsdata1((unsigned char*)p->data, p->length, false);
		stOnFootData ONFOOT1;
		bool ppassData = false;

		bsdata1.IgnoreBits(8);
		bsdata1.Read(OyuncuID);

		memset(&ONFOOT1, 0, sizeof(ONFOOT1));

		bool bHasLR, bHasUD;
		bool bHasSurfInfo, bAnimation;

		// LEFT/RIGHT KEYS
		bsdata1.Read(bHasLR);
		if (bHasLR) bsdata1.Read(ONFOOT1.sLeftRightKeys);

		// UP/DOWN KEYS
		bsdata1.Read(bHasUD);
		if (bHasUD) bsdata1.Read(ONFOOT1.sUpDownKeys);

		// GENERAL KEYS
		bsdata1.Read(ONFOOT1.sKeys);

		// VECTOR POS
		bsdata1.Read(ONFOOT1.fPosition[0]);
		bsdata1.Read(ONFOOT1.fPosition[1]);
		bsdata1.Read(ONFOOT1.fPosition[2]);

		bsdata1.ReadNormQuat(
			ONFOOT1.fQuaternion[0],
			ONFOOT1.fQuaternion[1],
			ONFOOT1.fQuaternion[2],
			ONFOOT1.fQuaternion[3]);

		// HEALTH/ARMOUR (COMPRESSED INTO 1 BYTE)
		BYTE byteHealthArmour;
		BYTE byteHealth, byteArmour;
		BYTE byteArmTemp = 0, byteHlTemp = 0;

		bsdata1.Read(byteHealthArmour);
		byteArmTemp = (byteHealthArmour & 0x0F);
		byteHlTemp = (byteHealthArmour >> 4);

		if (byteArmTemp == 0xF) byteArmour = 100;
		else if (byteArmTemp == 0) byteArmour = 0;
		else byteArmour = byteArmTemp * 7;

		if (byteHlTemp == 0xF) byteHealth = 100;
		else if (byteHlTemp == 0) byteHealth = 0;
		else byteHealth = byteHlTemp * 7;

		ONFOOT1.byteHealth = byteHealth;
		ONFOOT1.byteArmor = byteArmour;

		bsdata1.Read(ONFOOT1.byteCurrentWeapon);

		bsdata1.Read(ONFOOT1.byteSpecialAction);
		int act = bsdata1.GetReadOffset();

		bsdata1.ReadVector(
			ONFOOT1.fMoveSpeed[0],
			ONFOOT1.fMoveSpeed[1],
			ONFOOT1.fMoveSpeed[2]);

		if (bHasSurfInfo)
		{
			bsdata1.Read(ONFOOT1.sSurfingVehicleID);
			bsdata1.ReadVector(
				ONFOOT1.fSurfingOffsets[0],
				ONFOOT1.fSurfingOffsets[1],
				ONFOOT1.fSurfingOffsets[2]);
		}
		else ONFOOT1.sSurfingVehicleID = -1;


		if (ONFOOT1.fMoveSpeed[0] >= 0.9 || ONFOOT1.fMoveSpeed[1] >= 0.9 || ONFOOT1.fMoveSpeed[2] >= 0.9)
		{

			float zerospeed;
			bsdata1.SetWriteOffset(act);
			bsdata1.WriteVector(zerospeed, zerospeed, zerospeed);
			cheat_state_text("{c8c800}[anticheat] {ffffff}ID: %i [%s] {c8c800} airbrake kullaniyor! kullandigi fmovespeed: %f", OyuncuID, getPlayerName(OyuncuID), ONFOOT1.fMoveSpeed[2]);
		}

	}

	while (p != nullptr)
	{
		if (OnReceivePacket(p))
			break;
		g_RakClient->GetInterface()->DeallocatePacket(p);
		p = g_RakClient->GetInterface()->Receive();
	}
	return p;
}

bool HookedRakClientInterface::Connect(const char* host, unsigned short serverPort, unsigned short clientPort, unsigned int depreciated, int threadSleepTimer)
{
	traceLastFunc("HookedRakClientInterface::Connect");

	return g_RakClient->GetInterface()->Connect(host, serverPort, clientPort, depreciated, threadSleepTimer);
}

void HookedRakClientInterface::Disconnect(unsigned int blockDuration, unsigned char orderingChannel)
{
	traceLastFunc("HookedRakClientInterface::Disconnect");

	g_RakClient->GetInterface()->Disconnect(blockDuration, orderingChannel);
}

void HookedRakClientInterface::InitializeSecurity(const char* privKeyP, const char* privKeyQ)
{
	traceLastFunc("HookedRakClientInterface::InitializeSecurity");

	g_RakClient->GetInterface()->InitializeSecurity(privKeyP, privKeyQ);
}

void HookedRakClientInterface::SetPassword(const char* _password)
{
	traceLastFunc("HookedRakClientInterface::SetPassword");

	g_RakClient->GetInterface()->SetPassword(_password);
}

bool HookedRakClientInterface::HasPassword(void) const
{
	traceLastFunc("HookedRakClientInterface::HasPassword");

	return g_RakClient->GetInterface()->HasPassword();
}

bool HookedRakClientInterface::Send(const char* data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel)
{
	traceLastFunc("HookedRakClientInterface::Send");

	return g_RakClient->GetInterface()->Send(data, length, priority, reliability, orderingChannel);
}

void HookedRakClientInterface::DeallocatePacket(Packet* packet)
{
	traceLastFunc("HookedRakClientInterface::DeallocatePacket");

	g_RakClient->GetInterface()->DeallocatePacket(packet);
}

void HookedRakClientInterface::PingServer(void)
{
	traceLastFunc("HookedRakClientInterface::PingServer");

	g_RakClient->GetInterface()->PingServer();
}

void HookedRakClientInterface::PingServer(const char* host, unsigned short serverPort, unsigned short clientPort, bool onlyReplyOnAcceptingConnections)
{
	traceLastFunc("HookedRakClientInterface::PingServer");

	g_RakClient->GetInterface()->PingServer(host, serverPort, clientPort, onlyReplyOnAcceptingConnections);
}

int HookedRakClientInterface::GetAveragePing(void)
{
	traceLastFunc("HookedRakClientInterface::GetAveragePing");

	return g_RakClient->GetInterface()->GetAveragePing();
}

int HookedRakClientInterface::GetLastPing(void) const
{
	traceLastFunc("HookedRakClientInterface::GetLastPing");

	return g_RakClient->GetInterface()->GetLastPing();
}

int HookedRakClientInterface::GetLowestPing(void) const
{
	traceLastFunc("HookedRakClientInterface::GetLowestPing");

	return g_RakClient->GetInterface()->GetLowestPing();
}

int HookedRakClientInterface::GetPlayerPing(const PlayerID playerId)
{
	traceLastFunc("HookedRakClientInterface::GetPlayerPing");

	return g_RakClient->GetInterface()->GetPlayerPing(playerId);
}

void HookedRakClientInterface::StartOccasionalPing(void)
{
	traceLastFunc("HookedRakClientInterface::StartOccasionalPing");

	g_RakClient->GetInterface()->StartOccasionalPing();
}

void HookedRakClientInterface::StopOccasionalPing(void)
{
	traceLastFunc("HookedRakClientInterface::StopOccasionalPing");

	g_RakClient->GetInterface()->StopOccasionalPing();
}

bool HookedRakClientInterface::IsConnected(void) const
{
	traceLastFunc("HookedRakClientInterface::IsConnected");

	return g_RakClient->GetInterface()->IsConnected();
}

unsigned int HookedRakClientInterface::GetSynchronizedRandomInteger(void) const
{
	traceLastFunc("HookedRakClientInterface::GetSynchronizedRandomInteger");

	return g_RakClient->GetInterface()->GetSynchronizedRandomInteger();
}

bool HookedRakClientInterface::GenerateCompressionLayer(unsigned int inputFrequencyTable[256], bool inputLayer)
{
	traceLastFunc("HookedRakClientInterface::GenerateCompressionLayer");

	return g_RakClient->GetInterface()->GenerateCompressionLayer(inputFrequencyTable, inputLayer);
}

bool HookedRakClientInterface::DeleteCompressionLayer(bool inputLayer)
{
	traceLastFunc("HookedRakClientInterface::DeleteCompressionLayer");

	return g_RakClient->GetInterface()->DeleteCompressionLayer(inputLayer);
}

void HookedRakClientInterface::RegisterAsRemoteProcedureCall(int* uniqueID, void(*functionPointer) (RPCParameters* rpcParms))
{
	traceLastFunc("HookedRakClientInterface::RegisterAsRemoteProcedureCall");

	g_RakClient->GetInterface()->RegisterAsRemoteProcedureCall(uniqueID, functionPointer);
}

void HookedRakClientInterface::RegisterClassMemberRPC(int* uniqueID, void* functionPointer)
{
	traceLastFunc("HookedRakClientInterface::RegisterClassMemberRPC");

	g_RakClient->GetInterface()->RegisterClassMemberRPC(uniqueID, functionPointer);
}

void HookedRakClientInterface::UnregisterAsRemoteProcedureCall(int* uniqueID)
{
	traceLastFunc("HookedRakClientInterface::UnregisterAsRemoteProcedureCall");

	g_RakClient->GetInterface()->UnregisterAsRemoteProcedureCall(uniqueID);
}

bool HookedRakClientInterface::RPC(int* uniqueID, const char* data, unsigned int bitLength, PacketPriority priority, PacketReliability reliability, char orderingChannel, bool shiftTimestamp)
{
	traceLastFunc("HookedRakClientInterface::RPC");

	return g_RakClient->GetInterface()->RPC(uniqueID, data, bitLength, priority, reliability, orderingChannel, shiftTimestamp);
}

void HookedRakClientInterface::SetTrackFrequencyTable(bool b)
{
	traceLastFunc("HookedRakClientInterface::SetTrackFrequencyTable");

	g_RakClient->GetInterface()->SetTrackFrequencyTable(b);
}

bool HookedRakClientInterface::GetSendFrequencyTable(unsigned int outputFrequencyTable[256])
{
	traceLastFunc("HookedRakClientInterface::GetSendFrequencyTable");

	return g_RakClient->GetInterface()->GetSendFrequencyTable(outputFrequencyTable);
}

float HookedRakClientInterface::GetCompressionRatio(void) const
{
	traceLastFunc("HookedRakClientInterface::GetCompressionRatio");

	return g_RakClient->GetInterface()->GetCompressionRatio();
}

float HookedRakClientInterface::GetDecompressionRatio(void) const
{
	traceLastFunc("HookedRakClientInterface::GetDecompressionRatio");

	return g_RakClient->GetInterface()->GetDecompressionRatio();
}

void HookedRakClientInterface::AttachPlugin(void* messageHandler)
{
	traceLastFunc("HookedRakClientInterface::AttachPlugin");

	g_RakClient->GetInterface()->AttachPlugin(messageHandler);
}

void HookedRakClientInterface::DetachPlugin(void* messageHandler)
{
	traceLastFunc("HookedRakClientInterface::DetachPlugin");

	g_RakClient->GetInterface()->DetachPlugin(messageHandler);
}

BitStream* HookedRakClientInterface::GetStaticServerData(void)
{
	traceLastFunc("HookedRakClientInterface::GetStaticServerData");

	return g_RakClient->GetInterface()->GetStaticServerData();
}

void HookedRakClientInterface::SetStaticServerData(const char* data, const int length)
{
	traceLastFunc("HookedRakClientInterface::SetStaticServerData");

	g_RakClient->GetInterface()->SetStaticServerData(data, length);
}

BitStream* HookedRakClientInterface::GetStaticClientData(const PlayerID playerId)
{
	traceLastFunc("HookedRakClientInterface::GetStaticClientData");

	return g_RakClient->GetInterface()->GetStaticClientData(playerId);
}

void HookedRakClientInterface::SetStaticClientData(const PlayerID playerId, const char* data, const int length)
{
	traceLastFunc("HookedRakClientInterface::SetStaticClientData");

	g_RakClient->GetInterface()->SetStaticClientData(playerId, data, length);
}

void HookedRakClientInterface::SendStaticClientDataToServer(void)
{
	traceLastFunc("HookedRakClientInterface::SendStaticClientDataToServer");

	g_RakClient->GetInterface()->SendStaticClientDataToServer();
}

PlayerID HookedRakClientInterface::GetServerID(void) const
{
	traceLastFunc("HookedRakClientInterface::GetServerID");

	return g_RakClient->GetInterface()->GetServerID();
}

PlayerID HookedRakClientInterface::GetPlayerID(void) const
{
	traceLastFunc("HookedRakClientInterface::GetPlayerID");

	return g_RakClient->GetInterface()->GetPlayerID();
}

PlayerID HookedRakClientInterface::GetInternalID(void) const
{
	traceLastFunc("HookedRakClientInterface::GetInternalID");

	return g_RakClient->GetInterface()->GetInternalID();
}

const char* HookedRakClientInterface::PlayerIDToDottedIP(const PlayerID playerId) const
{
	traceLastFunc("HookedRakClientInterface::PlayerIDToDottedIP");

	return g_RakClient->GetInterface()->PlayerIDToDottedIP(playerId);
}

void HookedRakClientInterface::PushBackPacket(Packet* packet, bool pushAtHead)
{
	traceLastFunc("HookedRakClientInterface::PushBackPacket");

	g_RakClient->GetInterface()->PushBackPacket(packet, pushAtHead);
}

void HookedRakClientInterface::SetRouterInterface(void* routerInterface)
{
	traceLastFunc("HookedRakClientInterface::SetRouterInterface");

	g_RakClient->GetInterface()->SetRouterInterface(routerInterface);
}
void HookedRakClientInterface::RemoveRouterInterface(void* routerInterface)
{
	traceLastFunc("HookedRakClientInterface::RemoveRouterInterface");

	g_RakClient->GetInterface()->RemoveRouterInterface(routerInterface);
}

void HookedRakClientInterface::SetTimeoutTime(RakNetTime timeMS)
{
	traceLastFunc("HookedRakClientInterface::SetTimeoutTime");

	g_RakClient->GetInterface()->SetTimeoutTime(timeMS);
}

bool HookedRakClientInterface::SetMTUSize(int size)
{
	traceLastFunc("HookedRakClientInterface::SetMTUSize");

	return g_RakClient->GetInterface()->SetMTUSize(size);
}

int HookedRakClientInterface::GetMTUSize(void) const
{
	traceLastFunc("HookedRakClientInterface::GetMTUSize");

	return g_RakClient->GetInterface()->GetMTUSize();
}

void HookedRakClientInterface::AllowConnectionResponseIPMigration(bool allow)
{
	traceLastFunc("HookedRakClientInterface::AllowConnectionResponseIPMigration");

	g_RakClient->GetInterface()->AllowConnectionResponseIPMigration(allow);
}

void HookedRakClientInterface::AdvertiseSystem(const char* host, unsigned short remotePort, const char* data, int dataLength)
{
	traceLastFunc("HookedRakClientInterface::AdvertiseSystem");

	g_RakClient->GetInterface()->AdvertiseSystem(host, remotePort, data, dataLength);
}

RakNetStatisticsStruct* const HookedRakClientInterface::GetStatistics(void)
{
	traceLastFunc("HookedRakClientInterface::GetStatistics");

	return g_RakClient->GetInterface()->GetStatistics();
}

void HookedRakClientInterface::ApplyNetworkSimulator(double maxSendBPS, unsigned short minExtraPing, unsigned short extraPingVariance)
{
	traceLastFunc("HookedRakClientInterface::ApplyNetworkSimulator");

	g_RakClient->GetInterface()->ApplyNetworkSimulator(maxSendBPS, minExtraPing, extraPingVariance);
}

bool HookedRakClientInterface::IsNetworkSimulatorActive(void)
{
	traceLastFunc("HookedRakClientInterface::IsNetworkSimulatorActive");

	return g_RakClient->GetInterface()->IsNetworkSimulatorActive();
}

PlayerIndex HookedRakClientInterface::GetPlayerIndex(void)
{
	traceLastFunc("HookedRakClientInterface::GetPlayerIndex");

	return g_RakClient->GetInterface()->GetPlayerIndex();
}

bool HookedRakClientInterface::RPC_(int* uniqueID, BitStream* bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, bool shiftTimestamp, NetworkID networkID)
{
	traceLastFunc("HookedRakClientInterface::RPC_");

	return g_RakClient->GetInterface()->RPC_(uniqueID, bitStream, priority, reliability, orderingChannel, shiftTimestamp, networkID);
}