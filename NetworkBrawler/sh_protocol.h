#pragma once

#include <Sel/Color.hpp>
#include <Sel/Vector2.hpp>
#include <enet6/enet.h>
#include "sh_constants.h"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// Ce fichier contient tout ce qui va être lié au protocole du jeu, à la façon dont le client et le serveur vont communiquer

enum class Opcode : std::uint8_t
{
	C_PlayerName,
	C_CreateShapeRequest,
	C_DeleteShapeRequest,
	S_CreateShape,
	S_DeleteShape,
	S_PlayerList,
	S_ShapeStates,

	C_CreateBrawlerRequest,
	S_CreateBrawler,
	S_BrawlerStates,
	S_DeleteBrawler
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

// Un joueur souhaite créer son brawler
struct CreateBrawlerPacket
{
	static constexpr Opcode opcode = Opcode::S_CreateBrawler;

	/*BrawlerData brawlerData;*/
	std::uint32_t brawlerId;
	Sel::Vector2f position;
	Sel::Vector2f linearVelocity;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static CreateBrawlerPacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

// Le serveur envoie à un client la liste de tous les joueurs connectés
struct PlayerListPacket
{
	static constexpr Opcode opcode = Opcode::S_PlayerList;

	struct Player
	{
		std::string name;
		Sel::Color color;
	};

	std::vector<Player> players;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static PlayerListPacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
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
struct DeleteBrawlerPacket
{
	static constexpr Opcode opcode = Opcode::S_DeleteBrawler;

	std::uint32_t brawlerId;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static DeleteBrawlerPacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
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