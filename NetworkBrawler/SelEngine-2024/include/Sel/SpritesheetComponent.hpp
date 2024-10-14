#pragma once

#include <Sel/Export.hpp>
#include <Sel/Spritesheet.hpp>
#include <Sel/Vector2.hpp>
#include <entt/fwd.hpp>
#include <nlohmann/json_fwd.hpp>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace Sel
{
	class AnimationSystem;
	class Sprite;
	class WorldEditor;

	// Composant permettant de gérer l'animation via spritesheet
	// C'est ici qu'on va stocker l'état actuel de l'animation (la frame, les infos de temps, ...)
	// Ainsi qu'une référence (shared_ptr) vers le sprite à changer.
	// On ne peut pas réutiliser le GraphicsComponent car il peut cibler autre chose qu'un Sprite (ou ne pas être présent)
	// Cela permet de dissocier les responsabilités (animation et rendu)
	class SEL_ENGINE_API SpritesheetComponent
	{
		friend AnimationSystem;

		public:
			SpritesheetComponent(std::shared_ptr<const Spritesheet> spritesheet, std::shared_ptr<Sprite> targetSprite);

			void PlayAnimation(const std::string& animName);
			void PlayAnimation(std::size_t animIndex);

			void PopulateInspector(WorldEditor& worldEditor);

			nlohmann::json Serialize(entt::handle entity) const;
			static void Unserialize(entt::handle entity, const nlohmann::json& doc);

		private:
			void Update(float elapsedTime);

			std::size_t m_currentAnimation;
			std::shared_ptr<Sprite> m_targetSprite;
			std::shared_ptr<const Spritesheet> m_spritesheet;
			float m_timeAccumulator;
			unsigned int m_currentFrameIndex;
	};
}