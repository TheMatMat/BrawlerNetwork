#include "sh_protocol.h"
#include <cassert>
#include <cstring>

void BrawlerData::Serialize(std::vector<std::uint8_t>& byteArray) const
{
}

BrawlerData BrawlerData::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	return BrawlerData();
}

void PlayerNamePacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_str(byteArray, name);
}

PlayerNamePacket PlayerNamePacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	PlayerNamePacket packet;

	packet.name = Deserialize_str(byteArray, offset);

	return packet;
}

void CreateBrawlerPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u32(byteArray, playerId);
	Serialize_u32(byteArray, brawlerId);
	Serialize_f32(byteArray, position.x);
	Serialize_f32(byteArray, position.y);
	Serialize_f32(byteArray, linearVelocity.x);
	Serialize_f32(byteArray, linearVelocity.y);
	Serialize_f32(byteArray, scale);
	Serialize_str(byteArray, brawlerName);
}

CreateBrawlerPacket CreateBrawlerPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	CreateBrawlerPacket packet;

	packet.playerId = Deserialize_u32(byteArray, offset);
	packet.brawlerId = Deserialize_u32(byteArray, offset);
	float posX = Deserialize_f32(byteArray, offset);
	float posY = Deserialize_f32(byteArray, offset);
	packet.position = Sel::Vector2(posX, posY);

	float velX = Deserialize_f32(byteArray, offset);
	float velY = Deserialize_f32(byteArray, offset);
	packet.linearVelocity = Sel::Vector2f(velX, velY);

	packet.scale = Deserialize_f32(byteArray, offset);

	packet.brawlerName = Deserialize_str(byteArray, offset);

	return packet;
}

void BrawlerStatesPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u32(byteArray, brawlers.size());
	for (const States& state : brawlers)
	{
		Serialize_u32(byteArray, state.brawlerId);

		Serialize_f32(byteArray, state.position.x);
		Serialize_f32(byteArray, state.position.y);

		Serialize_f32(byteArray, state.linearVelocity.x);
		Serialize_f32(byteArray, state.linearVelocity.y);
	}
}

BrawlerStatesPacket BrawlerStatesPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	BrawlerStatesPacket packet;

	packet.brawlers.resize(Deserialize_u32(byteArray, offset));

	for (auto& state : packet.brawlers)
	{
		state.brawlerId = Deserialize_u32(byteArray, offset);

		float posX = Deserialize_f32(byteArray, offset);
		float posY = Deserialize_f32(byteArray, offset);
		state.position = Sel::Vector2(posX, posY);

		float velX = Deserialize_f32(byteArray, offset);
		float velY = Deserialize_f32(byteArray, offset);
		state.linearVelocity = Sel::Vector2f(velX, velY);
	}

	return packet;
}

void DeleteEntityPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u32(byteArray, brawlerId);
}

DeleteEntityPacket DeleteEntityPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	DeleteEntityPacket packet;

	packet.brawlerId = Deserialize_u32(byteArray, offset);

	return packet;
}


void Serialize_color(std::vector<std::uint8_t>& byteArray, const Sel::Color& value)
{
	Serialize_f32(byteArray, value.r);
	Serialize_f32(byteArray, value.g);
	Serialize_f32(byteArray, value.b);
	Serialize_f32(byteArray, value.a);
}

void Serialize_f32(std::vector<std::uint8_t>& byteArray, float value)
{
	std::size_t offset = byteArray.size();
	byteArray.resize(offset + sizeof(value));

	return Serialize_f32(byteArray, offset, value);
}

void Serialize_f32(std::vector<std::uint8_t>& byteArray, std::size_t offset, float value)
{
	std::uint32_t v = htonf(value);

	assert(offset + sizeof(v) <= byteArray.size());
	std::memcpy(&byteArray[offset], &v, sizeof(v));
}

void Serialize_i8(std::vector<std::uint8_t>& byteArray, std::int8_t value)
{
	return Serialize_u8(byteArray, static_cast<std::uint8_t>(value));
}

void Serialize_i8(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::int8_t value)
{
	return Serialize_u8(byteArray, offset, static_cast<std::uint8_t>(value));
}

void Serialize_i16(std::vector<std::uint8_t>& byteArray, std::int16_t value)
{
	return Serialize_u16(byteArray, static_cast<std::uint16_t>(value));
}

void Serialize_i16(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::int16_t value)
{
	return Serialize_u16(byteArray, offset, static_cast<std::uint16_t>(value));
}

void Serialize_i32(std::vector<std::uint8_t>& byteArray, std::int32_t value)
{
	return Serialize_u32(byteArray, static_cast<std::uint32_t>(value));
}

void Serialize_i32(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::int32_t value)
{
	return Serialize_u32(byteArray, offset, static_cast<std::uint32_t>(value));
}

void Serialize_u8(std::vector<std::uint8_t>& byteArray, std::uint8_t value)
{
	std::size_t offset = byteArray.size();
	byteArray.resize(offset + sizeof(value));

	return Serialize_u8(byteArray, offset, value);
}

void Serialize_u8(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::uint8_t value)
{
	assert(offset < byteArray.size());
	byteArray[offset] = value;
}

void Serialize_u16(std::vector<std::uint8_t>& byteArray, std::uint16_t value)
{
	std::size_t offset = byteArray.size();
	byteArray.resize(offset + sizeof(value));

	return Serialize_u16(byteArray, offset, value);
}

void Serialize_u16(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::uint16_t value)
{
	value = htons(value);

	assert(offset + sizeof(value) <= byteArray.size());
	std::memcpy(&byteArray[offset], &value, sizeof(value));
}

void Serialize_u32(std::vector<std::uint8_t>& byteArray, std::uint32_t value)
{
	std::size_t offset = byteArray.size();
	byteArray.resize(offset + sizeof(value));

	return Serialize_u32(byteArray, offset, value);
}

void Serialize_u32(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::uint32_t value)
{
	value = htonl(value);

	assert(offset + sizeof(value) <= byteArray.size());
	std::memcpy(&byteArray[offset], &value, sizeof(value));
}

void Serialize_str(std::vector<std::uint8_t>& byteArray, const std::string& value)
{
	std::size_t offset = byteArray.size();
	byteArray.resize(offset + sizeof(std::uint32_t) + value.size());
	return Serialize_str(byteArray, offset, value);
}

void Serialize_str(std::vector<std::uint8_t>& byteArray, std::size_t offset, const std::string& value)
{
	Serialize_u32(byteArray, offset, static_cast<std::uint32_t>(value.size()));
	offset += sizeof(std::uint32_t);

	if (!value.empty())
		std::memcpy(&byteArray[offset], value.data(), value.size());
}


Sel::Color Deserialize_color(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	Sel::Color value;
	value.r = Deserialize_f32(byteArray, offset);
	value.g = Deserialize_f32(byteArray, offset);
	value.b = Deserialize_f32(byteArray, offset);
	value.a = Deserialize_f32(byteArray, offset);

	return value;
}

float Deserialize_f32(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	std::uint32_t value;
	std::memcpy(&value, &byteArray[offset], sizeof(value));

	float v = ntohf(value);

	offset += sizeof(value);

	return v;
}

std::int8_t Deserialize_i8(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	return static_cast<std::int8_t>(Deserialize_u8(byteArray, offset));
}

std::int16_t Deserialize_i16(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	return static_cast<std::int16_t>(Deserialize_u16(byteArray, offset));
}

std::int32_t Deserialize_i32(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	return static_cast<std::int32_t>(Deserialize_u32(byteArray, offset));
}

std::uint8_t Deserialize_u8(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	std::uint8_t value = byteArray[offset];
	offset += sizeof(value);

	return value;
}

std::uint16_t Deserialize_u16(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	std::uint16_t value;
	std::memcpy(&value, &byteArray[offset], sizeof(value));
	value = ntohs(value);

	offset += sizeof(value);

	return value;
}

std::uint32_t Deserialize_u32(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	std::uint32_t value;
	std::memcpy(&value, &byteArray[offset], sizeof(value));
	value = ntohl(value);

	offset += sizeof(value);

	return value;
}

std::string Deserialize_str(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	std::uint32_t length = Deserialize_u32(byteArray, offset);
	std::string str(length, ' ');
	std::memcpy(&str[0], &byteArray[offset], length);

	offset += length;

	return str;
}

void PlayerListPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u16(byteArray, players.size());

	for (const auto& player : players)
	{
		Serialize_u32(byteArray, player.id);
		Serialize_str(byteArray, player.name);
		Serialize_u8(byteArray, player.isDead);
		Serialize_u8(byteArray, player.hasBrawler);
		if (player.hasBrawler)
			Serialize_u32(byteArray, player.brawlerId.value());
	}
}

PlayerListPacket PlayerListPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	PlayerListPacket packet;
	packet.players.resize(Deserialize_u16(byteArray, offset));

	for (auto& player : packet.players)
	{
		player.id = Deserialize_u32(byteArray, offset);
		player.name = Deserialize_str(byteArray, offset);
		player.isDead = Deserialize_u8(byteArray, offset);
		player.hasBrawler = Deserialize_u8(byteArray, offset);
		if (player.hasBrawler)
			player.brawlerId = Deserialize_u32(byteArray, offset);
	}

	return packet;
}

void CreateBrawlerResquest::Serialize(std::vector<std::uint8_t>& byteArray) const
{
}

CreateBrawlerResquest CreateBrawlerResquest::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	return CreateBrawlerResquest();
}

void UpdateSelfBrawlerId::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u32(byteArray, id);
}

UpdateSelfBrawlerId UpdateSelfBrawlerId::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	UpdateSelfBrawlerId packet;

	packet.id = Deserialize_u32(byteArray, offset);

	return packet;
}

void PlayerInputsPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u32(byteArray, brawlerId);
	Serialize_u8(byteArray, inputs.moveLeft);
	Serialize_u8(byteArray, inputs.moveRight);
	Serialize_u8(byteArray, inputs.moveUp);
	Serialize_u8(byteArray, inputs.moveDown);
}

PlayerInputsPacket PlayerInputsPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	PlayerInputsPacket packet;
	packet.brawlerId = Deserialize_u32(byteArray, offset);

	packet.inputs.moveLeft = Deserialize_u8(byteArray, offset);
	packet.inputs.moveRight = Deserialize_u8(byteArray, offset);
	packet.inputs.moveUp = Deserialize_u8(byteArray, offset);
	packet.inputs.moveDown = Deserialize_u8(byteArray, offset);

	return packet;
}

void CreateCollectiblePacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u32(byteArray, collectibleId);
	Serialize_f32(byteArray, position.x);
	Serialize_f32(byteArray, position.y);
	Serialize_f32(byteArray, scale);
	Serialize_u8(byteArray, static_cast<std::uint8_t>(type));
}

CreateCollectiblePacket CreateCollectiblePacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	CreateCollectiblePacket packet;

	packet.collectibleId = Deserialize_u32(byteArray, offset);

	float posX = Deserialize_f32(byteArray, offset);
	float posY = Deserialize_f32(byteArray, offset);
	packet.position = Sel::Vector2f(posX, posY);

	packet.scale = Deserialize_f32(byteArray, offset);
	packet.type = static_cast<CollectibleType>(Deserialize_u8(byteArray, offset));

	return packet;
}

void CollectibleCollectedPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
}

CollectibleCollectedPacket CollectibleCollectedPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	return CollectibleCollectedPacket();
}

void UpdateGameStatePacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u8(byteArray, newGameState);
}

UpdateGameStatePacket UpdateGameStatePacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	UpdateGameStatePacket packet;
	
	packet.newGameState = Deserialize_u8(byteArray, offset);

	return packet;
}


void PlayerReadyPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u8(byteArray, newReadyValue);
}

PlayerReadyPacket PlayerReadyPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	PlayerReadyPacket packet;
	
	packet.newReadyValue = Deserialize_u8(byteArray, offset);

	return packet;
}

void PlayerStealPacketRequest::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u32(byteArray, brawlerId);
}

PlayerStealPacketRequest PlayerStealPacketRequest::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	PlayerStealPacketRequest packet;

	packet.brawlerId = Deserialize_u32(byteArray, offset);

	return packet;
}

void WinnerPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u32(byteArray, brawlerNetworkId);
}

WinnerPacket WinnerPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	WinnerPacket packet;

	packet.brawlerNetworkId = Deserialize_u32(byteArray, offset);

	return packet;
}

void UpdateLeaderboardPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u32(byteArray, leaderboard.size());
	for (auto& data : leaderboard)
	{
		Serialize_u32(byteArray, data.playerId);
		Serialize_str(byteArray, data.playerName);
		Serialize_u32(byteArray, data.playerScore);
		Serialize_u8(byteArray, data.isDead);
	}
}

UpdateLeaderboardPacket UpdateLeaderboardPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	UpdateLeaderboardPacket packet;

	packet.leaderboard.resize(Deserialize_u32(byteArray, offset));

	for (auto& data : packet.leaderboard)
	{
		data.playerId = Deserialize_u32(byteArray, offset);
		data.playerName = Deserialize_str(byteArray, offset);
		data.playerScore = Deserialize_u32(byteArray, offset);
		data.isDead =Deserialize_u8(byteArray, offset);
	}

	return packet;
}

void UpdatePlayerModePacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u8(byteArray, newPlayerMode);
}

UpdatePlayerModePacket UpdatePlayerModePacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	UpdatePlayerModePacket packet;

	packet.newPlayerMode = Deserialize_u8(byteArray, offset);

	return packet;
}

void BrawlerDeathPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u32(byteArray, playerId);
	Serialize_u32(byteArray, brawlerId);
	Serialize_f32(byteArray, deathPosition.x);
	Serialize_f32(byteArray, deathPosition.y);
	Serialize_i8(byteArray, deathScaleX);
}

BrawlerDeathPacket BrawlerDeathPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	BrawlerDeathPacket packet;

	packet.playerId = Deserialize_u32(byteArray, offset);
	packet.brawlerId = Deserialize_u32(byteArray, offset);

	float deathX = Deserialize_f32(byteArray, offset);
	float deathY = Deserialize_f32(byteArray, offset);
	packet.deathPosition = { deathX, deathY };

	packet.deathScaleX = Deserialize_i8(byteArray, offset);

	return packet;
}

void PlayerStealPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u32(byteArray, brawlerId);
}

PlayerStealPacket PlayerStealPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	PlayerStealPacket packet;

	packet.brawlerId = Deserialize_u32(byteArray, offset);

	return packet;
}
