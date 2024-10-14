#pragma once

#include <Sel/Color.hpp>
#include <Sel/Export.hpp>
#include <Sel/Renderable.hpp>
#include <Sel/Vector2.hpp>
#include <SDL2/SDL.h>
#include <nlohmann/json_fwd.hpp> //< header spécial qui fait des déclarations anticipées des classes de la lib
#include <memory>
#include <string>

namespace Sel
{
	class Renderer;
	class Texture;

	class SEL_ENGINE_API Sprite : public Renderable
	{
		public:
			Sprite(std::shared_ptr<Texture> texture);
			Sprite(std::shared_ptr<Texture> texture, const SDL_Rect& rect);

			void Draw(Renderer& renderer, const Matrix3f& transformMatrix) const override;

			SDL_FRect GetBounds() const override;
			Color GetColor() const;

			void PopulateInspector(WorldEditor& worldEditor) override;

			void Resize(int width, int height);

			void SetColor(const Color& color);
			void SetOrigin(const Vector2f& origin);
			void SetRect(const SDL_Rect& rect);

			void SaveToFile(const std::string& filepath) const;
			nlohmann::json SaveToJSon() const;

			nlohmann::json Serialize() const override;

			static Sprite LoadFromFile(const std::string& filepath);
			static Sprite LoadFromJSon(const nlohmann::json& spriteDoc);
			static std::shared_ptr<Sprite> Unserialize(const nlohmann::json& spriteDoc);

		private:
			std::shared_ptr<Texture> m_texture;
			std::string m_texturePath; // Editor only
			Color m_color;
			Vector2f m_origin;
			SDL_Rect m_rect;
			int m_height;
			int m_width;
	};
}