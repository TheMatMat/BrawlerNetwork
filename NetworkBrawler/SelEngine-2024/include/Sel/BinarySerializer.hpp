#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Sel
{
	template<typename T> void SerializeBinary(std::vector<std::uint8_t>& byteArray, T value);
	void SerializeBinary(std::vector<std::uint8_t>& byteArray, const std::string& value);

	template<typename T> T UnserializeBinary(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
	template<> std::string UnserializeBinary(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
}

#include <Sel/BinarySerializer.inl>
