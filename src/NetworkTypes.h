#ifndef __NETWORK_TYPES_H
#define __NETWORK_TYPES_H

/// Forward declaration
namespace RakNet
{
	class BitStream;
};

struct RPCParameters;

#define BITS_TO_BYTES(x) (((x)+7)>>3)
#define BYTES_TO_BITS(x) ((x)<<3)

typedef unsigned short PlayerIndex;
typedef unsigned char RPCIndex;
const int MAX_RPC_MAP_SIZE = ((RPCIndex)-1) - 1;

using RPCFunction = void(*)(RPCParameters* p);

#ifdef __GET_TIME_64BIT
typedef long long RakNetTime;
typedef long long RakNetTimeNS;
#else
typedef unsigned int RakNetTime;
typedef long long RakNetTimeNS;
#endif

#pragma pack(push, 1)

struct PlayerID
{
	unsigned int binaryAddress;
	unsigned short port;
};

struct NetworkID
{
	PlayerID playerId;
	unsigned short localSystemId;
};

struct Packet
{
	PlayerIndex playerIndex;
	PlayerID playerId;
	unsigned int length;
	unsigned int bitSize;
	unsigned char* data;
	bool deleteData;
};

struct RPCParameters
{
	unsigned char* input;
	unsigned int numberOfBitsOfData;
	PlayerID sender;
};

const PlayerID UNASSIGNED_PLAYER_ID = { 0xFFFFFFFF, 0xFFFF };

#pragma pack(pop)

#endif