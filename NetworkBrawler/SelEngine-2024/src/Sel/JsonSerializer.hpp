#pragma once

#include <Sel/Vector2.hpp>
#include <nlohmann/json.hpp>
#include <SDL2/SDL_rect.h>

inline void from_json(const nlohmann::json& j, SDL_Rect& rect)
{
	rect.x = j.at("x");
	rect.y = j.at("y");
	rect.w = j.at("w");
	rect.h = j.at("h");
}

inline void to_json(nlohmann::json& j, const SDL_Rect& rect)
{
	j["x"] = rect.x;
	j["y"] = rect.y;
	j["w"] = rect.w;
	j["h"] = rect.h;
}

namespace Sel
{
	template<typename T>
	void from_json(const nlohmann::json& j, Vector2<T>& vec)
	{
		vec.x = j.at("x");
		vec.y = j.at("y");
	}

	template<typename T>
	void to_json(nlohmann::json& j, const Vector2<T>& vec)
	{
		j["x"] = vec.x;
		j["y"] = vec.y;
	}
}
