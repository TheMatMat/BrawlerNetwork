#pragma once

#include <Sel/Color.hpp>
#include <Sel/Vector2.hpp>
#include <enet6/enet.h>
#include "sh_constants.h"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include "sh_inputs.h"

// Ce fichier contient tout ce qui va être lié au protocole du jeu, à la façon dont le client et le serveur vont communiquer

enum class Opcode : std::uint8_t
{
	C_PlayerName,
	/*C_CreateShapeRequest,
	C_DeleteShapeRequest,
	S_CreateShape,
	S_DeleteShape,
	S_ShapeStates,*/

	S_PlayerList,
	C_CreateBrawlerRequest,
	C_PlayerInputs,
	C_PlayerReady,
	S_CreateBrawler,
	S_CreateCollectible,
	S_BrawlerStates,
	S_DeleteBrawler,
	S_UpdateSelfBrawlerId,
	S_UpdateGameState,
	S_UpdatePlayerMode,
	S_CollectibleCollected,
	S_UpdateLeaderboard,
	S_BrawlerDeath,
	S_Winner,
};

struct BrawlerFlag
{
	std::uint32_t playerId;
};

struct CollectibleFlag
{
	CollectibleType type;
};

struct DeadFlag
{

};

struct LeaderBoardLine
{

};

struct LeaderBoardData
{
	ENetPeer* peer;
	std::uint16_t score;
	bool isDead;
};

// Comme plusieurs paquets vont s'envoyer des informations de shapes, on peut les réunir
struct BrawlerData
{
	Sel::Color color;
	Sel::Vector2f position;
	Sel::Vector2f linearVelocity;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static BrawlerData Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

// Un joueur souhaite créer un Brawler
struct CreateBrawlerResquest
{
	static constexpr Opcode opcode = Opcode::C_CreateBrawlerRequest;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static CreateBrawlerResquest Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

// Un joueur souhaite renseigner son nom
struct PlayerNamePacket
{
	static constexpr Opcode opcode = Opcode::C_PlayerName;

	std::string name;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static PlayerNamePacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

struct PlayerReadyPacket
{
	static constexpr Opcode opcode = Opcode::C_PlayerReady;

	bool newReadyValue;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static PlayerReadyPacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

struct UpdateGameStatePacket
{
	static constexpr Opcode opcode = Opcode::S_UpdateGameState;

	std::uint8_t newGameState;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static UpdateGameStatePacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

// Le serveur indique un changement de mode de jeu au client (playing, dead, spectating)
struct UpdatePlayerModePacket
{
	static constexpr Opcode opcode = Opcode::S_UpdatePlayerMode;

	std::uint8_t newPlayerMode;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static UpdatePlayerModePacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

// Le serveur indique la mort d'un brawler
struct BrawlerDeathPacket
{
	static constexpr Opcode opcode = Opcode::S_BrawlerDeath;

	std::uint32_t playerId;
	std::uint32_t brawlerId;
	Sel::Vector2f deathPosition;
	std::int8_t deathScaleX;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static BrawlerDeathPacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

// Le serveur indique la création d'un brawler
struct CreateBrawlerPacket
{
	static constexpr Opcode opcode = Opcode::S_CreateBrawler;
	
	std::uint32_t playerId;
	std::uint32_t brawlerId;
	Sel::Vector2f position;
	Sel::Vector2f linearVelocity;
	float scale;
	std::string brawlerName;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static CreateBrawlerPacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

// Le serveur notifie d'un winner
struct WinnerPacket
{
	static constexpr Opcode opcode = Opcode::S_Winner;

	std::uint32_t brawlerNetworkId;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static WinnerPacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

// Le serveur indique la création d'un collectible
struct CreateCollectiblePacket
{
	static constexpr Opcode opcode = Opcode::S_CreateCollectible;

	/*BrawlerData brawlerData;*/
	std::uint32_t collectibleId;
	Sel::Vector2f position;
	float scale;
	CollectibleType type;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static CreateCollectiblePacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};


// Un joueur envois ses inputs au serveur
struct PlayerInputsPacket
{
	static constexpr Opcode opcode = Opcode::C_PlayerInputs;

	std::uint32_t brawlerId;
	PlayerInputs inputs;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static PlayerInputsPacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

// Le serveur envoie à un client la liste de tous les joueurs connectés
struct PlayerListPacket
{
	static constexpr Opcode opcode = Opcode::S_PlayerList;

	struct Player
	{
		std::uint32_t id;
		std::string name;
		bool isDead;
		bool hasBrawler;
		std::optional<std::uint32_t> brawlerId;
	};

	std::vector<Player> players;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static PlayerListPacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

// Le serveur notifie le player du networkId de son propre brawler
struct UpdateSelfBrawlerId
{
	static constexpr Opcode opcode = Opcode::S_UpdateSelfBrawlerId;

	std::uint32_t id;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static UpdateSelfBrawlerId Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

// Le serveur indique au joueur qu'il a recup un collectible
struct CollectibleCollectedPacket
{
	static constexpr Opcode opcode = Opcode::S_CollectibleCollected;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static CollectibleCollectedPacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

// Le serveur indique aux joueurs un changement de leaderboard
struct UpdateLeaderboardPacket
{
	static constexpr Opcode opcode = Opcode::S_UpdateLeaderboard;

	struct Data
	{
		std::uint32_t playerId;
		std::string playerName;
		std::uint32_t playerScore;
		bool isDead;
	};

	std::vector<Data> leaderboard; // first to last

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static UpdateLeaderboardPacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

// Le serveur envoie les données sur tous les brawlers
struct BrawlerStatesPacket
{
	static constexpr Opcode opcode = Opcode::S_BrawlerStates;

	struct States
	{
		std::uint32_t brawlerId;
		Sel::Vector2f position;
		Sel::Vector2f linearVelocity;
	};

	std::vector<States> brawlers;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static BrawlerStatesPacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

// Le serveur annonce qu'un brawler cesse d'exister
struct DeleteEntityPacket
{
	static constexpr Opcode opcode = Opcode::S_DeleteBrawler;

	std::uint32_t brawlerId;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static DeleteEntityPacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

void Serialize_color(std::vector<std::uint8_t>& byteArray, const Sel::Color& value);
void Serialize_f32(std::vector<std::uint8_t>& byteArray, float value);
void Serialize_f32(std::vector<std::uint8_t>& byteArray, std::size_t offset, float value);
void Serialize_i8(std::vector<std::uint8_t>& byteArray, std::int8_t value);
void Serialize_i8(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::int8_t value);
void Serialize_i16(std::vector<std::uint8_t>& byteArray, std::int16_t value);
void Serialize_i16(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::int16_t value);
void Serialize_i32(std::vector<std::uint8_t>& byteArray, std::int32_t value);
void Serialize_i32(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::int32_t value);
void Serialize_u8(std::vector<std::uint8_t>& byteArray, std::uint8_t value);
void Serialize_u8(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::uint8_t value);
void Serialize_u16(std::vector<std::uint8_t>& byteArray, std::uint16_t value);
void Serialize_u16(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::uint16_t value);
void Serialize_u32(std::vector<std::uint8_t>& byteArray, std::uint32_t value);
void Serialize_u32(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::uint32_t value);
void Serialize_str(std::vector<std::uint8_t>& byteArray, const std::string& value);
void Serialize_str(std::vector<std::uint8_t>& byteArray, std::size_t offset, const std::string& value);

Sel::Color Deserialize_color(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
float Deserialize_f32(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
std::int8_t Deserialize_i8(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
std::int16_t Deserialize_i16(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
std::int32_t Deserialize_i32(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
std::uint8_t Deserialize_u8(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
std::uint16_t Deserialize_u16(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
std::uint32_t Deserialize_u32(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
std::string Deserialize_str(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);

// Petite fonction d'aide pour construire un packet ENet à partir d'une de nos structures de packet, insère automatiquement l'opcode au début des données
template<typename T> ENetPacket* build_packet(const T& packet, enet_uint32 flags)
{
	// On sérialise l'opcode puis le contenu du packet dans un std::vector<std::uint8_t>
	std::vector<std::uint8_t> byteArray;

	Serialize_u8(byteArray, static_cast<std::uint8_t>(T::opcode));
	packet.Serialize(byteArray);

	// On copie le contenu de ce vector dans un packet enet, et on l'envoie au peer
	return enet_packet_create(byteArray.data(), byteArray.size(), flags);
}