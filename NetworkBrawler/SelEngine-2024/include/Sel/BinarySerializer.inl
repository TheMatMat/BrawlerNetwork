#include <cstring>

namespace Sel
{
	template<typename T>
	void SerializeBinary(std::vector<std::uint8_t>& byteArray, T value)
	{
		std::size_t offset = byteArray.size();
		byteArray.resize(offset + sizeof(value));
		std::memcpy(&byteArray[offset], &value, sizeof(value));
	}

	void SerializeBinary(std::vector<std::uint8_t>& byteArray, const std::string& value)
	{
		SerializeBinary(byteArray, static_cast<std::uint16_t>(value.size()));

		std::size_t offset = byteArray.size();
		byteArray.resize(offset + value.size());
		std::memcpy(&byteArray[offset], &value[0], value.size());
	}

	template<typename T>
	T UnserializeBinary(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
	{
		if (offset + sizeof(T) > byteArray.size())
			throw std::runtime_error("unserialization error: buffer is too small");

		T value;
		std::memcpy(&value, &byteArray[offset], sizeof(T));
		offset += sizeof(T);

		return value;
	}

	template<>
	std::string UnserializeBinary(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
	{
		std::uint16_t strLength = UnserializeBinary<std::uint16_t>(byteArray, offset);
		if (offset + strLength > byteArray.size())
			throw std::runtime_error("unserialization error: buffer is too small");

		std::string value(strLength, '\0');
		std::memcpy(&value[0], &byteArray[offset], strLength);
		offset += strLength;

		return value;
	}
}
