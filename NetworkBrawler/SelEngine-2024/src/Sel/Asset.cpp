#include <Sel/Asset.hpp>

namespace Sel
{
	Asset::Asset(std::string filepath) :
	m_filepath(std::move(filepath))
	{
	}

	Asset::Asset(Asset&& asset) noexcept :
	m_filepath(std::move(asset.m_filepath))
	{
	}

	const std::string& Asset::GetFilepath() const
	{
		return m_filepath;
	}

	Asset& Asset::operator=(Asset&& asset) noexcept
	{
		std::swap(m_filepath, asset.m_filepath);
		return *this;
	}
}