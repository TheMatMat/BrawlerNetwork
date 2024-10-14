#include <Sel/SpritesheetComponent.hpp>
#include <Sel/GraphicsComponent.hpp>
#include <Sel/ResourceManager.hpp>
#include <Sel/Sprite.hpp>
#include <entt/entt.hpp>
#include <fmt/color.h>
#include <fmt/core.h>
#include <nlohmann/json.hpp>
#include <imgui.h>
#include <SDL.h> //< SDL_Rect

namespace Sel
{
	SpritesheetComponent::SpritesheetComponent(std::shared_ptr<const Spritesheet> spritesheet, std::shared_ptr<Sprite> targetSprite) :
	m_currentAnimation(0),
	m_targetSprite(std::move(targetSprite)),
	m_spritesheet(std::move(spritesheet)),
	m_timeAccumulator(0.f),
	m_currentFrameIndex(0)
	{
	}

	void SpritesheetComponent::PlayAnimation(const std::string& animName)
	{
		auto indexOpt = m_spritesheet->GetAnimationByName(animName);
		if (!indexOpt.has_value())
		{
			fmt::print(stderr, fg(fmt::color::red), "animation \"{}\" not found\n", animName);
			return;
		}

		PlayAnimation(indexOpt.value());
	}

	void SpritesheetComponent::PlayAnimation(std::size_t animIndex)
	{
		if (m_currentAnimation == animIndex)
			return;

		m_currentAnimation = animIndex;
		m_currentFrameIndex = 0;
		m_timeAccumulator = 0.f;
	}

	void SpritesheetComponent::PopulateInspector(WorldEditor& /*worldEditor*/)
	{
		const char* anim = m_spritesheet->GetAnimation(m_currentAnimation).name.c_str();
		if (ImGui::BeginCombo("Animation", anim))
		{
			for (std::size_t i = 0; i < m_spritesheet->GetAnimationCount(); ++i)
			{
				bool isSelected = (i == m_currentAnimation);
				if (ImGui::Selectable(m_spritesheet->GetAnimation(i).name.c_str()))
					PlayAnimation(i);

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		ImGui::Text("Current frame: %u", m_currentFrameIndex);
	}

	nlohmann::json SpritesheetComponent::Serialize(const entt::handle entity) const
	{
		// Le cas que nous souhaitons sauvegarder est celui où nous disposons d'un graphics component avec le même target sprite que nous
		// (c'est un problème qui aurait pu être évité si nous avions directement utilisé le sprite comme composant 😇)
		// Heureusement nous avons la certitude d'être sérialisé/déserialisé après le graphics component (car enregistré après dans le ComponentRegistry)

		nlohmann::json doc;
		doc["SpritesheetPath"] = m_spritesheet->GetFilepath();

		if (const GraphicsComponent* gfxEntity = entity.try_get<GraphicsComponent>())
		{
			if (gfxEntity->renderable == m_targetSprite)
			{
				// On indique que le renderable est bien notre sprite
				doc["SpriteGfx"] = true;
			}
		}

		return doc;
	}

	void SpritesheetComponent::Unserialize(entt::handle entity, const nlohmann::json& doc)
	{
		GraphicsComponent* gfxEntity = entity.try_get<GraphicsComponent>();
		if (gfxEntity && doc["SpriteGfx"] == true)
		{
			std::shared_ptr<const Spritesheet> spriteSheet = ResourceManager::Instance().GetSpritesheet(doc["SpritesheetPath"]);
			std::shared_ptr<Sprite> targetSprite = std::static_pointer_cast<Sprite>(gfxEntity->renderable); // static_cast mais pour les std::shared_ptr

			entity.emplace<SpritesheetComponent>(std::move(spriteSheet), std::move(targetSprite));
		}
	}

	void SpritesheetComponent::Update(float elapsedTime)
	{
		if (m_currentAnimation >= m_spritesheet->GetAnimationCount())
			return; //< Peut arriver si aucune animation n'a été ajoutée

		const Spritesheet::Animation& anim = m_spritesheet->GetAnimation(m_currentAnimation);

		m_timeAccumulator += elapsedTime;
		while (m_timeAccumulator >= anim.frameDuration)
		{
			m_timeAccumulator -= anim.frameDuration;

			// On fait boucler l'animation
			if (++m_currentFrameIndex >= anim.frameCount)
				m_currentFrameIndex = 0;

			SDL_Rect rect;
			rect.x = anim.start.x + anim.size.x * m_currentFrameIndex;
			rect.y = anim.start.y;
			rect.w = anim.size.x;
			rect.h = anim.size.y;

			m_targetSprite->SetRect(rect);
		}
	}
}
