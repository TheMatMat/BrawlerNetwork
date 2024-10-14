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
}

CreateBrawlerPacket CreateBrawlerPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	return CreateBrawlerPacket();
}

void BrawlerStatesPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
}

BrawlerStatesPacket BrawlerStatesPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	return BrawlerStatesPacket();
}

void DeleteBrawlerPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
}

DeleteBrawlerPacket DeleteBrawlerPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	return DeleteBrawlerPacket();
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
}

PlayerListPacket PlayerListPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	return PlayerListPacket();
}
