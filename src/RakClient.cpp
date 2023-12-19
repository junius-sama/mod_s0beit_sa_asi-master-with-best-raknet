/*

	PROJECT:		mod_sa
	LICENSE:		See LICENSE in the top level directory
	COPYRIGHT:		Copyright we_sux, BlastHack

	mod_sa is available from https://github.com/BlastH1ckNet/mod_s0b1it_sa/

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

RakClient* g_RakClient = NULL;

RakClient::RakClient(void* pRakClientInterface)
{
	pRakClient = (RakClientInterface*)pRakClientInterface;
}

bool RakClient::RPC(int rpcId, BitStream* bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, bool shiftTimestamp)
{
	if (!pRakClient)
		return false;

	return pRakClient->RPC(&rpcId, bitStream, priority, reliability, orderingChannel, shiftTimestamp);
}

bool RakClient::Send(BitStream* bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel)
{
	if (!pRakClient)
		return false;

	return pRakClient->Send(bitStream, priority, reliability, orderingChannel);
}

// misc functions
void RakClient::SendDeath(uint16_t killerId, uint8_t reason)
{
	BitStream bsDeath;

	bsDeath.Write(reason);
	bsDeath.Write(killerId);
	g_RakClient->RPC(RPC_Death, &bsDeath);
}

void RakClient::SendPickUp(int pickupId)
{
	BitStream bsPickup;

	bsPickup.Write(pickupId);

	RPC(RPC_PickedUpPickup, &bsPickup);
}

void RakClient::RequestClass(int classId)
{
	BitStream bsClass;

	bsClass.Write(classId);
	g_RakClient->RPC(RPC_RequestClass, &bsClass);
}

void RakClient::SendSCMEvent(int vehicleID, int eventId, int param1, int param2)
{
	BitStream bsScmEvent;

	bsScmEvent.Write(vehicleID);
	bsScmEvent.Write(param1);
	bsScmEvent.Write(param2);
	bsScmEvent.Write(eventId);

	RPC(RPC_SCMEvent, &bsScmEvent);
}
void RakClient::GivePlayerWeapon(uint16_t playerid, BYTE weaponid, WORD ammo)
{
	BitStream bsWep;
	bsWep.Write(playerid);
	bsWep.Write(weaponid);
	bsWep.Write(ammo);
	g_RakClient->RPC(RPC_GivePlayerWeapon, &bsWep, HIGH_PRIORITY, RELIABLE, 0, 0);
}
void RakClient::SendSpawn(void)
{
	BitStream bsSpawn;

	g_RakClient->RPC(RPC_RequestSpawn, &bsSpawn);
	g_RakClient->RPC(RPC_Spawn, &bsSpawn);
}

void RakClient::SendGiveDamage(uint16_t uiPlayerID, float fDamageAmountToSend, int iWeaponID, int iBodyPart)
{
	BitStream bsGiveDamage;
	bsGiveDamage.Write(false);
	bsGiveDamage.Write((UINT16)uiPlayerID);
	bsGiveDamage.Write(fDamageAmountToSend);
	bsGiveDamage.Write((UINT32)iWeaponID);
	bsGiveDamage.Write(iBodyPart);
	g_RakClient->RPC(RPC_GiveTakeDamage, &bsGiveDamage, MEDIUM_PRIORITY, UNRELIABLE, 0, 0);
}

void RakClient::SendAnormal(uint16_t uiPlayerID, float fDamageAmountToSend, int iWeaponID)
{
	BitStream bsGiveDamage;
	bsGiveDamage.Write(false);
	bsGiveDamage.Write((UINT16)uiPlayerID);
	bsGiveDamage.Write(fDamageAmountToSend);
	bsGiveDamage.Write((UINT32)iWeaponID);
	g_RakClient->RPC(RPC_GiveTakeDamage, &bsGiveDamage, MEDIUM_PRIORITY, UNRELIABLE, 0, 0);
}

void RakClient::SendDialogTextResponse(WORD wDialogID, BYTE bButtonID, WORD wListBoxItem, const char* szInputResp)
{

	BYTE respLen = (BYTE)strlen(szInputResp);
	BitStream bsSend;
	bsSend.Write(wDialogID);
	bsSend.Write(bButtonID);
	bsSend.Write(wListBoxItem);
	bsSend.Write(respLen);
	bsSend.Write(szInputResp, respLen);
	g_RakClient->RPC(62, &bsSend, SYSTEM_PRIORITY, UNRELIABLE, 0, FALSE);
}

void RakClient::SendDialogResponse(WORD wDialogID, BYTE bButtonID, WORD wListBoxItem)
{
	BitStream bsSend;
	bsSend.Write(wDialogID);
	bsSend.Write(bButtonID);
	bsSend.Write(wListBoxItem);
	g_RakClient->RPC(62, &bsSend, SYSTEM_PRIORITY, UNRELIABLE, 0, FALSE);
}