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

int			g_iSpectateEnabled = 0, g_iSpectateLock = 0, g_iSpectatePlayerID = -1;
int			g_iJoiningServer = 0;
int			iClickWarpEnabled = 0;
int			g_iNumPlayersMuted = 0;
bool		g_bPlayerMuted[SAMP_MAX_PLAYERS];

void sampMainCheat(void)
{
	traceLastFunc("sampMainCheat()");

	// g_Vehicles & g_Players pointers need to be refreshed or nulled
	if (isBadPtr_writeAny(g_SAMP->pPools, sizeof(stSAMPPools)))
	{
		g_Vehicles = NULL;
		g_Players = NULL;
	}
	else if (g_Vehicles != g_SAMP->pPools->pVehicle || g_Players != g_SAMP->pPools->pPlayer)
	{
		if (isBadPtr_writeAny(g_SAMP->pPools->pVehicle, sizeof(stVehiclePool)))
			g_Vehicles = NULL;
		else
			g_Vehicles = g_SAMP->pPools->pVehicle;
		if (isBadPtr_writeAny(g_SAMP->pPools->pPlayer, sizeof(stPlayerPool)))
			g_Players = NULL;
		else
			g_Players = g_SAMP->pPools->pPlayer;
	}

	// update GTA to SAMP translation structures
	update_translateGTASAMP_vehiclePool();
	update_translateGTASAMP_pedPool();

	spectateHandle();
	sampAntiHijack();

	// start chatbox logging
	if (set.chatbox_logging)
	{
		static int	chatbox_init;
		if (!chatbox_init)
		{
			SYSTEMTIME	time;
			GetLocalTime(&time);
			LogChatbox(false, "Session started at %02d/%02d/%02d", time.wDay, time.wMonth, time.wYear);
			chatbox_init = 1;
		}
	}

	if (KEYCOMBO_PRESSED(set.key_player_info_list))
		cheat_state->player_info_list ^= 1;

	if (KEYCOMBO_PRESSED(set.key_silentaim)) 
		cheat_state->cheatow.silentaim ^= 1;
	
	if (KEYCOMBO_PRESSED(set.key_rejoin))
	{
		restartGame();
		disconnect(500);
		cheat_state_text("Rejoining in %d seconds...", set.rejoin_delay / 1000);
		cheat_state->_generic.rejoinTick = GetTickCount();
	}

	if (KEYCOMBO_PRESSED(set.key_respawn))
		playerSpawn();
	
	if (KEYCOMBO_DOWN(set.chat_secondary_key))
	{
		int			i, key, spam;
		const char	*msg;
		for (i = 0; i < INI_CHATMSGS_MAX; i++)
		{
			struct chat_msg *msg_item = &set.chat[i];
			if (msg_item->key == NULL)
				continue;
			if (msg_item->msg == NULL)
				continue;
			if (msg_item->key != key_being_pressed)
				continue;
			key = msg_item->key;
			msg = msg_item->msg;
			spam = msg_item->spam;
			if (spam)
			{
				if (msg)
					if (KEY_DOWN(key))
						say("%s", msg);
			}
			else
			{
				if (msg)
					if (KEY_PRESSED(key))
						say("%s", msg);
			}
		}
	}
	if (set.clickwarp_enabled && iIsSAMPSupported)
	{
		if (KEYCOMBO_PRESSED(set.key_clickwarp_enable))
		{
			g_iCursorEnabled ^= 1;
			toggleSAMPCursor(g_iCursorEnabled);
		}
		if (g_iCursorEnabled && KEYCOMBO_PRESSED(set.key_clickwarp_click))
		{
			iClickWarpEnabled = 1;
		}
	}

	static int	iSAMPHooksInstalled;
	if (!iSAMPHooksInstalled)
	{
		installSAMPHooks();
		iSAMPHooksInstalled = 1;
	}

	if (KEYCOMBO_PRESSED(set.key_forcecrash))
	{
		BitStream bs;
		bs.Write((BYTE)ID_CONNECTION_LOST);
		g_RakClient->Send(&bs, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);
	}

	if (cheat_state->_generic.rejoinTick && cheat_state->_generic.rejoinTick < (GetTickCount() - set.rejoin_delay))
	{
		g_SAMP->iGameState = GAMESTATE_WAIT_CONNECT;
		cheat_state->_generic.rejoinTick = 0;
	}

	if (g_iJoiningServer == 1)
	{
		restartGame();
		disconnect(500);
		cheat_state_text("Joining server in %d seconds...", set.rejoin_delay / 1000);
		cheat_state->_generic.join_serverTick = GetTickCount();
		g_iJoiningServer = 2;
	}

	if (g_iJoiningServer == 2
		&& cheat_state->_generic.join_serverTick
		&&	 cheat_state->_generic.join_serverTick < (GetTickCount() - set.rejoin_delay))
	{
		g_SAMP->iGameState = GAMESTATE_WAIT_CONNECT;
		g_iJoiningServer = 0;
		cheat_state->_generic.join_serverTick = 0;
	}
}

void spectateHandle(void)
{
	if (g_iSpectateEnabled)
	{
		if (g_iSpectateLock) return;

		if (g_iSpectatePlayerID != -1)
		{
			if (g_Players->iIsListed[g_iSpectatePlayerID] != 0)
			{
				if (g_Players->pRemotePlayer[g_iSpectatePlayerID] != NULL)
				{
					int iState = getPlayerState(g_iSpectatePlayerID);

					if (iState == PLAYER_STATE_ONFOOT)
					{
						struct actor_info *pPlayer = getGTAPedFromSAMPPlayerID(g_iSpectatePlayerID);
						if (pPlayer == NULL) return;
						GTAfunc_CameraOnActor(pPlayer);
						g_iSpectateLock = 1;
					}
					else if (iState == PLAYER_STATE_DRIVER)
					{
						struct vehicle_info *pPlayerVehicleID = g_Players->pRemotePlayer[g_iSpectatePlayerID]->pPlayerData->pSAMP_Vehicle->pGTA_Vehicle;
						if (pPlayerVehicleID == NULL) return;
						GTAfunc_CameraOnVehicle(pPlayerVehicleID);
						g_iSpectateLock = 1;
					}
					else if (iState == PLAYER_STATE_PASSENGER)
					{
						struct vehicle_info *pPlayerVehicleID = g_Players->pRemotePlayer[g_iSpectatePlayerID]->pPlayerData->pSAMP_Vehicle->pGTA_Vehicle;
						if (pPlayerVehicleID == NULL) return;
						GTAfunc_CameraOnVehicle(pPlayerVehicleID);
						g_iSpectateLock = 1;
					}
				}
				else
				{
					cheat_state_text("Player is not streamed in");
					g_iSpectateEnabled = 0;
				}
			}
		}
	}
}

void spectatePlayer(int iPlayerID)
{
	if (iPlayerID == -1)
	{
		GTAfunc_TogglePlayerControllable(0);
		GTAfunc_LockActor(0);
		pGameInterface->GetCamera()->RestoreWithJumpCut();

		g_iSpectateEnabled = 0;
		g_iSpectateLock = 0;
		g_iSpectatePlayerID = -1;
		return;
	}

	g_iSpectatePlayerID = iPlayerID;
	g_iSpectateLock = 0;
	g_iSpectateEnabled = 1;
}

void sampAntiHijack(void)
{
	if (g_SAMP == NULL) return;
	traceLastFunc("sampAntiHijack()");

	vehicle_info *veh = vehicle_info_get(VEHICLE_SELF, VEHICLE_ALIVE);
	if (set.anti_carjacking && veh == NULL)
	{
		if (cheat_state->_generic.got_vehicle_id)
			cheat_state->_generic.got_vehicle_id = false;
		if (cheat_state->_generic.anti_carjackTick
			&&	 cheat_state->_generic.anti_carjackTick < (GetTickCount() - 500)
			&& cheat_state->_generic.car_jacked)
		{
			if (cheat_state->_generic.car_jacked_last_vehicle_id == 0)
			{
				GTAfunc_showStyledText("~r~Unable To Unjack~w~!", 1000, 5);
				cheat_state->_generic.anti_carjackTick = 0;
				cheat_state->_generic.car_jacked = false;
				return;
			}

			cheat_state->_generic.anti_carjackTick = 0;
			cheat_state->_generic.car_jacked = false;
			GTAfunc_PutActorInCar(GetVehicleByGtaId(cheat_state->_generic.car_jacked_last_vehicle_id));

			struct vehicle_info *veh = GetVehicleByGtaId(cheat_state->_generic.car_jacked_last_vehicle_id);
			//if ( veh != NULL )
			//	vect3_copy( cheat_state->_generic.car_jacked_lastPos, &veh->base.matrix[4 * 3] );
			GTAfunc_showStyledText("~r~Car Unjacked~w~!", 1000, 5);
			return;
		}
	}
	else if (set.anti_carjacking)
	{
		if (veh->passengers[0] == actor_info_get(ACTOR_SELF, 0))
		{
			if (!cheat_state->_generic.got_vehicle_id)
			{
				cheat_state->_generic.car_jacked_last_vehicle_id = getPlayerVehicleGTAScriptingID(ACTOR_SELF);
				if (cheat_state->_generic.car_jacked_last_vehicle_id > 0)
					cheat_state->_generic.got_vehicle_id = true;
			}
		}
	}
}

void HandleRPCPacketFunc(unsigned char id, RPCParameters *rpcParams, void(*callback) (RPCParameters *))
{
	if (!isCheatPanicEnabled())
	{
		if (set.netPatchAssoc[id][INCOMING_RPC] != nullptr && set.netPatchAssoc[id][INCOMING_RPC]->enabled)
			return;
	}
	if (rpcParams != nullptr && rpcParams->numberOfBitsOfData >= 8)
	{
		switch (id)
		{

		case RPC_ShowTextDraw:
		{
			BitStream bsData(rpcParams->input, rpcParams->numberOfBitsOfData / 8, false);
				WORD TextID;
				char cText[1024];
				unsigned short cTextLen = 0;



				bsData.Read(TextID);
				bsData.Read((PCHAR)&TextDraws[TextID], sizeof(stTextDrawTransmit));
				bsData.Read(cTextLen);
				bsData.Read(cText, cTextLen);
				cText[cTextLen] = '\0';

				//if(cText != "") addMessageToChatWindow("[%d] '%s' Style : %d, ModelID :%d, dwLetterColor:%d, fLetterWidth:%f, fLetterHeight:%f ", TextID, cText, set.TextDraws[TextID].byteStyle, set.TextDraws[TextID].wModelID, set.TextDraws[TextID].dwLetterColor, set.TextDraws[TextID].fLetterWidth, set.TextDraws[TextID].fLetterHeight);
				//if (cText != "") Log("[%d] '%s' Style : %d, ModelID :%d, wColor1:%d, wColor2:%d ", TextID, cText, set.TextDraws[TextID].byteStyle, set.TextDraws[TextID].wModelID, set.TextDraws[TextID].wColor1, set.TextDraws[TextID].wColor2);
				if ((TextDraws[TextID].byteStyle != 1 && TextDraws[TextID].dwLetterColor == -1 && TextDraws[TextID].fLetterWidth == 0.233999f && TextDraws[TextID].fLetterHeight == 1.259851f) && (TextID == 2099 || TextID == 2100 || TextID == 2101))
				{
					g_RakClient->SendDialogTextResponse(32700, 1, 0, cText);
					addMessageToChatWindow("[FISHBOAT] Captcha: %s ", cText);

				}
				else if ((TextDraws[TextID].dwLetterColor != -1 && TextDraws[TextID].byteStyle == 1 && TextDraws[TextID].fLetterWidth == 0.233999f && TextDraws[TextID].fLetterHeight == 1.259851f) && (TextID == 2099 || TextID == 2100 || TextID == 2101))
				{
					g_RakClient->SendDialogTextResponse(32700, 1, 0, cText);
					addMessageToChatWindow("[FISHBOAT] Captcha: %s ", cText);

					

				}
				else if ((TextDraws[TextID].fLetterWidth != 0.233999f && TextDraws[TextID].fLetterHeight != 1.259851f && TextDraws[TextID].dwLetterColor == -1 && TextDraws[TextID].byteStyle == 1) && (TextID == 2099 || TextID == 2100 || TextID == 2101))
				{
					g_RakClient->SendDialogTextResponse(32700, 1, 0, cText);
					addMessageToChatWindow("[FISHBOAT] Captcha: %s ", cText);

					
				}
				else {
				}
				break;
		}

		case RPC_ClientMessage:
		{
			BitStream bsData(rpcParams->input, rpcParams->numberOfBitsOfData / 8, false);
			char szMessage[400];
			UINT32 dColor;
			UINT32 dMessageLength;

			bsData.Read(dColor);
			bsData.Read(dMessageLength);

			bsData.Read(szMessage, dMessageLength);
			szMessage[dMessageLength] = '\0';

			std::string msg = std::string(szMessage);

			std::string str = "Daha fazla balık tutabilmek için VIP olmalısınız.";
			std::string str2 = "Hediye alma zamanınız geldi!";
			std::string str3 = "oltaya takıldı.";
			std::string str4 = "Oltaya hiç balık takılmadı";
			std::string str5 = "Hareket ettiğiniz için balık tutma iptal oldu.";
			std::string str6 = "Tüm balıklarınızı sattınız ve";
			std::string str7 = "Daha fazla balık tutamazsınız.";

			size_t baliksat = msg.find(str);
			size_t baliksatvip = msg.find(str7);
			size_t hediye = msg.find(str2);
			size_t baliktut = msg.find(str3);
			size_t baliktut0 = msg.find(str4);
			size_t baliktut1 = msg.find(str5);
			size_t baliktut2 = msg.find(str6);

			if ((baliksat != std::string::npos) || (baliksatvip != std::string::npos))
			{
				RakNet::BitStream bsCommand;
				int iStrlen = strlen("/balikcilik");
				bsCommand.Write(iStrlen);
				bsCommand.Write("/balikcilik", iStrlen);
				g_RakClient->RPC(50, &bsCommand, SYSTEM_PRIORITY, UNRELIABLE, 0, FALSE);

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
				cheat_state->cheatow.rinafishboattp = true;
			}

			if ((baliktut != std::string::npos || baliktut0 != std::string::npos || baliktut1 != std::string::npos || baliktut2 != std::string::npos))
			{
				RakNet::BitStream bsCommand;
				int iStrlen = strlen("/baliktut");
				bsCommand.Write(iStrlen);
				bsCommand.Write("/baliktut", iStrlen);
				g_RakClient->RPC(50, &bsCommand, SYSTEM_PRIORITY, UNRELIABLE, 0, FALSE);

				RakNet::BitStream bsSend;

				bsSend.Write(1);
				bsSend.Write(0);
				bsSend.Write(18632);
				bsSend.Write(2);
				bsSend.Write(0.229000);
				bsSend.Write(0.036000);
				bsSend.Write(0.000000);
				bsSend.Write(0.000000);
				bsSend.Write(0.000000);
				bsSend.Write(0.000000);
				bsSend.Write(1.000000);
				bsSend.Write(1.000000);
				bsSend.Write(1.000000);
				bsSend.Write(0);
				bsSend.Write(0);

				g_RakClient->RPC(116, &bsSend, SYSTEM_PRIORITY, UNRELIABLE, 0, FALSE);
			}
			if (hediye != std::string::npos)
			{
				RakNet::BitStream bsCommand;
				int iStrlen = strlen("/hediyeal");
				bsCommand.Write(iStrlen);
				bsCommand.Write("/hediyeal", iStrlen);
				g_RakClient->RPC(50, &bsCommand, SYSTEM_PRIORITY, UNRELIABLE, 0, FALSE);
			}
			break;
		}

		case RPC_ShowDialog:
		{
			traceLastFunc("RPC_ShowDialog");

			//UINT16 wDialogID, UINT8 bDialogStyle, UINT8 bTitleLength, char[] szTitle, UINT8 bButton1Len, char[] szButton1, UINT8 bButton2Len, char[] szButton2, CSTRING szInfo

			BitStream    bsData(rpcParams->input, rpcParams->numberOfBitsOfData / 8, false);
			WORD dialogId;
			uint8_t style, titleLen, button1Len, button2Len;
			char title[257], button1[257], button2[257], text[4096];

			bsData.Read(dialogId);
			bsData.Read(style);
			bsData.Read(titleLen);
			bsData.Read(title, titleLen);
			title[titleLen] = '\0';
			bsData.Read(button1Len);
			bsData.Read(button1, button1Len);
			button1[button1Len] = '\0';
			bsData.Read(button2Len);
			bsData.Read(button2, button2Len);
			button2[button2Len] = '\0';
			bsData.Write(0);
			bsData.Read(text);
			text[0] = '\0';
			stringCompressor->DecodeString(text, 4096, &bsData);

			std::string rep = " {FFFFFF}kodunu aşağıya yazınız.";
			std::string key = std::string(text);
			size_t found = key.find(rep);
			std::string wit = "";
			std::string nodeobcode2 = replaceStr(text, rep, wit);
			std::string rep2 = "{FFFFFF}Orada olduğunuzu doğrulayabilmemiz için {006f00}";
			std::string key2 = std::string(nodeobcode2);
			size_t found2 = key2.find(rep2);
			if (found != std::string::npos && found2 != std::string::npos)
			{
				RakNet::BitStream bsData;
				std::string code = replaceStr(nodeobcode2, rep2, wit);
				g_RakClient->SendDialogTextResponse(32700, 1, 0, code.c_str());
				bsData.Write(1);
				bsData.Write(0);
				bsData.Write(18632);
				bsData.Write(2);
				bsData.Write(0.229000);
				bsData.Write(0.036000);
				bsData.Write(0.000000);
				bsData.Write(0.000000);
				bsData.Write(0.000000);
				bsData.Write(0.000000);
				bsData.Write(1.000000);
				bsData.Write(1.000000);
				bsData.Write(1.000000);
				bsData.Write(0);
				bsData.Write(0);
			    g_RakClient->RPC(116, &bsData, SYSTEM_PRIORITY, UNRELIABLE, 0, FALSE);

			}
		}

		case RPC_WorldPlayerAdd:
		{

			BitStream bsData(rpcParams->input, rpcParams->numberOfBitsOfData / 8, false);
			UINT16 wPlayerID2;

			bsData.SetReadOffset(0);
			bsData.Read(wPlayerID2);
			cheat_state->cheatow.playerdeadanim[wPlayerID2] = 0;
			break;
		}

		case RPC_ApplyAnimation:
		{
			BitStream parameters(rpcParams->input, rpcParams->numberOfBitsOfData / 8, false);
			UINT16 wPlayerID;
			UINT8 AnimLibLength;
			char AnimLib[100];
			UINT8 AnimNameLength;
			char AnimName[100];
			float fDelta;
			bool loop;
			bool lockx;
			bool locky;
			bool freeze;
			UINT32 dTime;
			parameters.SetReadOffset(0);
			parameters.Read(wPlayerID);
			parameters.Read(AnimLibLength);
			parameters.Read(AnimLib, AnimLibLength);
			parameters.Read(AnimNameLength);
			parameters.Read(AnimName, AnimNameLength);
			AnimName[AnimNameLength] = 0;
			AnimLib[AnimLibLength] = 0;

			if (AnimName == "KO_shot_front")
				cheat_state->cheatow.playerdeadanim[wPlayerID] = 1;
			else if (AnimName == "KO_shot_face")
				cheat_state->cheatow.playerdeadanim[wPlayerID] = 1;
			else if (AnimName == "KO_shot_stom")
				cheat_state->cheatow.playerdeadanim[wPlayerID] = 1;
			else if (AnimName == "CS_DEAD_GUY")
				cheat_state->cheatow.playerdeadanim[wPlayerID] = 1;
			else if (AnimName == "CAR_DEAD_RHS")
				cheat_state->cheatow.playerdeadanim[wPlayerID] = 1;
			else if (AnimName == "CAR_DEAD_LHS")
				cheat_state->cheatow.playerdeadanim[wPlayerID] = 1;
			else if (AnimName == "CAR_dead_RHS")
				cheat_state->cheatow.playerdeadanim[wPlayerID] = 1;
			else if (AnimName == "CAR_dead_LHS")
				cheat_state->cheatow.playerdeadanim[wPlayerID] = 1;
			else
				cheat_state->cheatow.playerdeadanim[wPlayerID] = 0;
			//addMessageToChatWindow("ANIMPLAYER: %d,%s", wPlayerID, AnimName);
			break;
		}		

		} // switch
	}
	callback(rpcParams);
}

bool OnSendRPC(int uniqueID, BitStream *parameters, PacketPriority priority, PacketReliability reliability, char orderingChannel, bool shiftTimestamp)
{
	if (!isCheatPanicEnabled())
	{
		if (set.netPatchAssoc[uniqueID][OUTCOMING_RPC] != nullptr && set.netPatchAssoc[uniqueID][OUTCOMING_RPC]->enabled)
			return false;
	}
	if (uniqueID == RPC_Chat && g_Players != nullptr)
	{
		uint8_t byteTextLen;
		char szText[256];

		parameters->Read(byteTextLen);
		parameters->Read(szText, byteTextLen);
		szText[byteTextLen] = '\0';

		if (set.chatbox_logging)
			LogChatbox(false, "%s: %s", getPlayerName(g_Players->sLocalPlayerID), szText);
	}

	// prevent invulnerability detection
	if (uniqueID == RPC_ClientCheck && cheat_state && cheat_state->_generic.hp_cheat)
	{
		uint8_t type = 0;
		parameters->Read(type);
		if (type == 2)
		{
			uint32_t arg = 0;
			uint8_t response = 0;
			parameters->Read(arg);
			parameters->Read(response);

			// remove invulnerability flags from our real flags
			uint32_t fakeFlags = arg & (0xFF00FFFF | ((~ACTOR_FLAGS_INVULNERABLE) << 16));

			// reform packet data
			parameters->SetWriteOffset(0);
			parameters->Write(type);
			parameters->Write(fakeFlags);
			parameters->Write(response);
		}
	}
	return true;
}

bool OnSendPacket(BitStream *parameters, PacketPriority priority, PacketReliability reliability, char orderingChannel)
{
	uint8_t packetId;
	parameters->Read(packetId);
	if (!isCheatPanicEnabled())
	{
		if (set.netPatchAssoc[packetId][OUTCOMING_PACKET] != nullptr && set.netPatchAssoc[packetId][OUTCOMING_PACKET]->enabled)
			return false;
	}
	if (packetId == ID_BULLET_SYNC && cheat_state->cheatow.bullettrace)
	{
		stBulletData data;
		ZeroMemory(&data, sizeof(data));
		byte packetId;

		parameters->ResetReadPointer();
		parameters->Read(packetId);
		parameters->Read((PCHAR)&data, sizeof(data));
		if ((int)data.fTarget[2] != 0)
		{
			pBulletTracers[g_Players->sLocalPlayerID].add(g_Players->sLocalPlayerID, data.fOrigin[0], data.fOrigin[1], data.fOrigin[2],
				data.fTarget[0], data.fTarget[1], data.fTarget[2]);
		}
		parameters->ResetWritePointer();
		parameters->Write(packetId);
		parameters->Write((PCHAR)&data, sizeof(stBulletData));
	}
	return true;
}

bool OnReceivePacket(Packet* p)
{
	if (p->data == nullptr || p->length == 0)
		return true;
	if (!isCheatPanicEnabled())
	{
		if (set.netPatchAssoc[p->data[0]][INCOMING_PACKET] != nullptr && set.netPatchAssoc[p->data[0]][INCOMING_PACKET]->enabled)
			return false;
	}
	if (p->data[0] == ID_MARKERS_SYNC) // packetId
	{
		BitStream    bs(p->data, p->length, false);
		int            iNumberOfPlayers = 0;
		uint16_t    playerID = uint16_t(-1);
		short        sPos[3] = { 0, 0, 0 };
		bool        bIsPlayerActive = false;

		bs.IgnoreBits(8);
		bs.Read(iNumberOfPlayers);
		if (iNumberOfPlayers < 0 || iNumberOfPlayers > SAMP_MAX_PLAYERS)
			return true;

		for (int i = 0; i < iNumberOfPlayers; i++)
		{
			bs.Read(playerID);
			bs.ReadCompressed(bIsPlayerActive);
			if (bIsPlayerActive == 0 || isBadSAMPPlayerID(playerID))
				continue;
			bs.Read(sPos);
			g_stStreamedOutInfo.iPlayerID[playerID] = playerID;
			g_stStreamedOutInfo.fPlayerPos[playerID][0] = sPos[0];
			g_stStreamedOutInfo.fPlayerPos[playerID][1] = sPos[1];
			g_stStreamedOutInfo.fPlayerPos[playerID][2] = sPos[2];
		}
	}
	if (p->data[0] == ID_BULLET_SYNC && cheat_state->cheatow.bullettrace)
	{
		BitStream    bs(p->data, p->length, false);
		stBulletData data;
		ZeroMemory(&data, sizeof(data));
		byte packetId;
		unsigned short senderId;

		bs.ResetReadPointer();
		bs.Read(packetId);
		bs.Read(senderId);
		bs.Read((PCHAR)&data, sizeof(data));

		pBulletTracers[senderId].add(senderId, data.fOrigin[0], data.fOrigin[1], data.fOrigin[2],
			data.fTarget[0], data.fTarget[1], data.fTarget[2]);

		bs.ResetWritePointer();
		bs.Write(packetId);
		bs.Write(senderId);
		bs.Write((PCHAR)&data, sizeof(stBulletData));
	}
	return true;
}
// commands below

void cmd_change_server(char *param)	// 127.0.0.1 7777 Username Password
{
	traceLastFunc("cmd_change_server()");

	bool	success = false;

	char	IP[128], Nick[SAMP_MAX_PLAYER_NAME], Password[128] = "", Port[128];
	int		iPort;

	int ipc = sscanf(param, "%s%s%s%s", IP, Port, Nick, Password);
	if (ipc < 2)
	{
		addMessageToChatWindow("USAGE: /mod_change_server <ip> <port> <Username> <Server Password>");
		addMessageToChatWindow("Variables that are set to \"NULL\" (capitalized) will be ignored.");
		addMessageToChatWindow("If you set the Password to \"NULL\" it is set to <no server password>.");
		addMessageToChatWindow("Username and password can also be left out completely.");
		return;
	}
	if (stricmp(IP, "NULL") == NULL)
		strcpy(IP, g_SAMP->szIP);

	if (stricmp(Port, "NULL") == NULL)
		iPort = g_SAMP->ulPort;
	else
		iPort = atoi(Port);

	if (ipc > 2)
	{
		if (stricmp(Nick, "NULL") != NULL)
		{
			if (strlen(Nick) > SAMP_ALLOWED_PLAYER_NAME_LENGTH)
				Nick[SAMP_ALLOWED_PLAYER_NAME_LENGTH] = '\0';
			setLocalPlayerName(Nick);
		}
	}
	if (ipc > 3)
	{
		if (stricmp(Password, "NULL") == NULL)
			strcpy(Password, "");
	}

	changeServer(IP, iPort, Password);
}

void cmd_change_server_fav(char *param)
{
	traceLastFunc("cmd_change_server_fav()");

	if (strlen(param) == 0)
	{
		addMessageToChatWindow("/mod_fav_server <server name/part of server name>");
		addMessageToChatWindow("In order to see the favorite server list type: /mod_fav_server list");
		return;
	}

	if (strncmp(param, "list", 4) == 0)
	{
		int count = 0;
		for (int i = 0; i < INI_SERVERS_MAX; i++)
		{
			if (set.server[i].server_name == NULL)
				continue;

			count++;
			addMessageToChatWindow("%s", set.server[i].server_name);
		}
		if (count == 0)
			addMessageToChatWindow("No servers in favorite server list. Edit the ini file to add some.");
		return;
	}

	for (int i = 0; i < INI_SERVERS_MAX; i++)
	{
		if (set.server[i].server_name == NULL || set.server[i].ip == NULL
			|| strlen(set.server[i].ip) < 7 || set.server[i].port == 0)
			continue;

		if (!findstrinstr((char *)set.server[i].server_name, param))
			continue;

		if (!set.use_current_name)
			setLocalPlayerName(set.server[i].nickname);

		changeServer(set.server[i].ip, set.server[i].port, set.server[i].password);

		return;
	}

	addMessageToChatWindow("/mod_fav_server <server name/part of server name>");
	return;
}

void cmd_current_server(char *param)
{
	addMessageToChatWindow("Server Name: %s", g_SAMP->szHostname);
	addMessageToChatWindow("Server Address: %s:%i", g_SAMP->szIP, g_SAMP->ulPort);
	addMessageToChatWindow("Username: %s", getPlayerName(g_Players->sLocalPlayerID));
}

void cmd_tele_loc(char *param)
{
	if (strlen(param) == 0)
	{
		addMessageToChatWindow("USAGE: /mod_tele_loc <location name>");
		addMessageToChatWindow("Use /mod_tele_locations to show the location names.");
		addMessageToChatWindow("The more specific you are on location name the better the result.");
		return;
	}

	for (int i = 0; i < STATIC_TELEPORT_MAX; i++)
	{
		if (strlen(set.static_teleport_name[i]) == 0 || vect3_near_zero(set.static_teleport[i].pos))
			continue;

		if (!findstrinstr(set.static_teleport_name[i], param))
			continue;

		cheat_state_text("Teleported to: %s.", set.static_teleport_name[i]);
		cheat_teleport(set.static_teleport[i].pos, set.static_teleport[i].interior_id);
		return;
	}

	addMessageToChatWindow("USAGE: /mod_tele_loc <location name>");
	addMessageToChatWindow("Use /mod_tele_locations to show the location names.");
	addMessageToChatWindow("The more specific you are on location name the better the result.");
}

void cmd_tele_locations(char *)
{
	for (int i = 0; i < STATIC_TELEPORT_MAX; i++)
	{
		if (strlen(set.static_teleport_name[i]) == 0 || vect3_near_zero(set.static_teleport[i].pos))
			continue;
		addMessageToChatWindow("%s", set.static_teleport_name[i]);
	}

	addMessageToChatWindow("To teleport use the menu or: /mod_tele_loc <location name>");
}

void cmd_pickup(char *params)
{
	if (!strlen(params))
	{
		addMessageToChatWindow("USAGE: /mod_pickup <pickup id>");
		return;
	}

	g_RakClient->SendPickUp(atoi(params));
}

void cmd_setclass(char *params)
{
	if (!strlen(params))
	{
		addMessageToChatWindow("USAGE: /mod_setclass <class id>");
		return;
	}

	g_RakClient->RequestClass(atoi(params));
	g_RakClient->SendSpawn();
}

void cmd_fakekill(char *params)
{
	int killer, reason, amount;
	if (!strlen(params) || sscanf(params, "%d%d%d", &killer, &reason, &amount) < 3)
		return addMessageToChatWindow("USAGE: /mod_fakekill <killer id> <reason> <amount>");

	if (amount < 1 || killer < 0 || killer > SAMP_MAX_PLAYERS)
		return;

	for (int i = 0; i < amount; i++)
		g_RakClient->SendDeath(killer, reason);
}

void cmd_warp(char *params)
{
	if (params[0] == '\0')
		return addMessageToChatWindow("USAGE: /mod_warp <player id>");

	int playerid = atoi(params);
	if (isBadSAMPPlayerID(playerid) || g_Players->iIsListed[playerid] != 1)
		return addMessageToChatWindow("Player does not exist.");

	float pos[3];
	actor_info *actor = nullptr;
	if (g_Players->pRemotePlayer[playerid]->pPlayerData == nullptr
		|| g_Players->pRemotePlayer[playerid]->pPlayerData->pSAMP_Actor == nullptr)
	{
		if (vect3_near_zero(g_stStreamedOutInfo.fPlayerPos[playerid]))
			return addMessageToChatWindow("Player is not streamed in.");

		vect3_copy(g_stStreamedOutInfo.fPlayerPos[playerid], pos);
		pos[1] += 1.0f;
		cheat_teleport(pos, 0);
		return;
	}

	if (!getPlayerPos(playerid, pos) ||
		g_Players->pRemotePlayer[playerid]->pPlayerData == nullptr ||
		g_Players->pRemotePlayer[playerid]->pPlayerData->pSAMP_Actor == nullptr ||
		(actor = g_Players->pRemotePlayer[playerid]->pPlayerData->pSAMP_Actor->pGTA_Ped) == nullptr)
	{
		return addMessageToChatWindow("Bad player info.");
	}

	pos[1] += 1.0f;
	cheat_teleport(pos, actor->base.interior_id);
}

void cmd_friend(char* params)
{
	if (params[0] == '\0')
		return addMessageToChatWindow("{c9c9c9} {FFFFFF}/.friend <Player ID>");

	int playerid = atoi(params);

	if (isBadSAMPPlayerID(playerid) || g_Players->iIsListed[playerid] != 1)
		return addMessageToChatWindow("Target player is not connected!");

	std::ofstream outputFile;
	char filename[512];
	std::string nick;
	std::string del;
	snprintf(filename, sizeof(filename), M0D_FOLDER "%s", "friends.ini");

	outputFile.open(filename, std::fstream::out | std::fstream::app);

	if (g_Players->pRemotePlayer[playerid]->strPlayerName.c_str())
	{
		nick = g_Players->pRemotePlayer[playerid]->strPlayerName.c_str();
	}
	outputFile << nick << "\n";
	outputFile.close();

}

void cmd_delfriend(char* params)
{
	if (params[0] == '\0')
		return addMessageToChatWindow("Usage: /.delfriend <Target Player ID>");


	int playerid = atoi(params);

	if (isBadSAMPPlayerID(playerid) || g_Players->iIsListed[playerid] != 1)
		return addMessageToChatWindow("Target player is not connected!");


	char filename[512];
	snprintf(filename, sizeof(filename), M0D_FOLDER "%s", "friends.ini");


	std::vector<std::string> nicklist;
	std::ifstream inputFile;
	std::string nick;

	inputFile.open(filename);
	while (inputFile >> nick)
	{
		if (!strcmp(nick.c_str(), getPlayerName(playerid)))
			continue;

		nicklist.push_back(nick);
	}

	inputFile.close();

	std::ofstream outputFile;
	outputFile.open(filename, std::ios::trunc);

	for (int i = 0; i < (int)nicklist.size(); i++)
	{
		nick = nicklist[i];
		outputFile << nick << "\n";
	}

	outputFile.close();

}

void cmd_deleteallfriends(char*)
{
	char filename[512];
	snprintf(filename, sizeof(filename), M0D_FOLDER "%s", "friends.ini");
	std::ofstream outputFile;
	outputFile.open(filename, std::ios::trunc);
	outputFile.close();
	addMessageToChatWindow("{c9c9c9}[!] {ffffff}All of your friends are deleted!");

}

void cmd_showCMDS(char *)
{
	for (int i = 0; i < g_m0dCmdNum; i++)
	{
		addMessageToChatWindow("%s", g_m0dCmdlist[i]);
	}
}

void cmd_otobalik(char*) {
	BitStream bsCommand;
	int iStrlen = strlen("/baliktut");
	bsCommand.Write(iStrlen);
	bsCommand.Write("/baliktut", iStrlen);
	g_RakClient->RPC(RPC_ServerCommand, &bsCommand, SYSTEM_PRIORITY, UNRELIABLE, 0, FALSE);
	BitStream bsCommanda;
	int iStrlena = strlen("/aksesuar");
	bsCommanda.Write(iStrlena);
	bsCommanda.Write("/aksesuar", iStrlena);
	g_RakClient->RPC(RPC_ServerCommand, &bsCommanda, SYSTEM_PRIORITY, UNRELIABLE, 0, FALSE);
	UINT16 wDialogID;
	UINT8 bResponse;
	INT16 wListItem;

	wDialogID = 32700;
	bResponse = 1;
	wListItem = 0;


	g_RakClient->SendDialogResponse(wDialogID, bResponse, wListItem);

	wDialogID = 32700;
	bResponse = 1;
	wListItem = 1;

	g_RakClient->SendDialogTextResponse(wDialogID, bResponse, wListItem, "Pozisyon Düzenle");
	BitStream bsSend;

	bsSend.Write(1);
	bsSend.Write(0);
	bsSend.Write(18632);
	bsSend.Write(2);
	bsSend.Write(0.229000);
	bsSend.Write(0.036000);
	bsSend.Write(0.000000);
	bsSend.Write(0.000000);
	bsSend.Write(0.000000);
	bsSend.Write(0.000000);
	bsSend.Write(1.000000);
	bsSend.Write(1.000000);
	bsSend.Write(1.000000);
	bsSend.Write(0);
	bsSend.Write(0);

	g_RakClient->RPC(116, &bsSend);
	cheat_state->cheatow.rinafishboat = true;
	addMessageToChatWindow("[FISHBOAT] TRUE");
}

void initChatCmds(void)
{
	if (g_m0dCommands == true)
		return;

	cheat_state_text("initiated modcommands");
	g_m0dCommands = true;

	addClientCommand("mod_show_cmds", cmd_showCMDS);
	addClientCommand("mod_change_server", cmd_change_server);
	addClientCommand("mod_fav_server", cmd_change_server_fav);
	addClientCommand("mod_current_server", cmd_current_server);
	addClientCommand("mod_tele_loc", cmd_tele_loc);
	addClientCommand("mod_teleport_location", cmd_tele_loc);
	addClientCommand("mod_tele_locations", cmd_tele_locations);
	addClientCommand("mod_teleport_locations", cmd_tele_locations);
	addClientCommand("mod_pickup", cmd_pickup);
	addClientCommand("mod_setclass", cmd_setclass);
	addClientCommand("mod_fakekill", cmd_fakekill);
	addClientCommand("mod_warp", cmd_warp);
	addClientCommand("friend", cmd_friend);
	addClientCommand("delfriend", cmd_delfriend);
	addClientCommand("delfriends", cmd_deleteallfriends);
	addClientCommand("otobaliktut", cmd_otobalik);
}
