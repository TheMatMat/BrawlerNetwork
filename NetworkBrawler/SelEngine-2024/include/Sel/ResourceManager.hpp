#pragma once

#include <Sel/Export.hpp>
#include <memory> //< std::shared_ptr
#include <string> //< std::string
#include <unordered_map> //< std::unordered_map est plus efficace que std::map pour une association clé/valeur

namespace Sel
{
	class Font;
	class Model;
	class Renderer;
	class Spritesheet;
	class Texture;
	
	class SEL_ENGINE_API ResourceManager
	{
		public:
			ResourceManager(Renderer& renderer);
			ResourceManager(const ResourceManager&) = delete;
			ResourceManager(ResourceManager&&) = delete;
			~ResourceManager();

			void Clear();

			const std::shared_ptr<Font>& GetFont(const std::string& fontPath);
			const std::shared_ptr<Model>& GetModel(const std::string& modelPath);
			const std::shared_ptr<Spritesheet>& GetSpritesheet(const std::string& spritesheetPath);
			const std::shared_ptr<Texture>& GetTexture(const std::string& texturePath);

			void Purge();

			static ResourceManager& Instance();

			ResourceManager& operator=(const ResourceManager&) = delete;
			ResourceManager& operator=(ResourceManager&&) = delete;

		private:
			std::shared_ptr<Model> m_missingModel;
			std::shared_ptr<Spritesheet> m_missingSpritesheet;
			std::shared_ptr<Texture> m_missingTexture;
			std::unordered_map<std::string /*fontPath*/, std::shared_ptr<Font>> m_fonts;
			std::unordered_map<std::string /*modelPath*/, std::shared_ptr<Model>> m_models;
			std::unordered_map<std::string /*spritesheetPath*/, std::shared_ptr<Spritesheet>> m_spritesheets;
			std::unordered_map<std::string /*texturePath*/, std::shared_ptr<Texture>> m_textures;
			Renderer& m_renderer;

			static ResourceManager* s_instance;
	};
}