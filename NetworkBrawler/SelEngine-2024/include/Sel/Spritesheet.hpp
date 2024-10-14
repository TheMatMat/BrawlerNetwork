#pragma once

#include <Sel/Asset.hpp>
#include <Sel/Vector2.hpp>
#include <filesystem>
#include <optional>
#include <unordered_map>
#include <vector>

namespace Sel
{
	class SEL_ENGINE_API Spritesheet : public Asset
	{
		public:
			struct Animation
			{
				std::string name;
				Vector2i size;
				Vector2i start;
				unsigned int frameCount;
				float frameDuration;
			};

			// On hérite du constructeur d'Asset (comme si on déclarait un constructeur de Spritesheet prenant les mêmes paramètres)
			using Asset::Asset;

			void AddAnimation(std::string name, unsigned int frameCount, float frameDuration, Vector2i start, Vector2i size);
			void AddAnimation(Animation animation);

			const Animation& GetAnimation(std::size_t animIndex) const;
			std::optional<std::size_t> GetAnimationByName(const std::string& animName) const;
			std::size_t GetAnimationCount() const;

			void SaveToFile(const std::string& filepath) const;

			static Spritesheet LoadFromFile(const std::string& filepath);

		private:
			std::unordered_map<std::string /*animName*/, std::size_t /*animIndex*/> m_animationByName;
			std::vector<Animation> m_animations;
	};
}
