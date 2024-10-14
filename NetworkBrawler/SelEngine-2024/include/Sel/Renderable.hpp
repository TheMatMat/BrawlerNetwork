#pragma once

#include <Sel/Export.hpp>
#include <nlohmann/json_fwd.hpp>

struct SDL_FRect;

namespace Sel
{
	class Renderer;
	class WorldEditor;

	// Déclaration anticipée de Matrix3 (classe template) et l'alias Matrix3f
	template<typename T> class Matrix3;
	using Matrix3f = Matrix3<float>;

	// Rappel: le C++ n'a pas de concept d'interface à proprement parler
	// à la place on utilise des classes possédant des méthodes virtuelles pures (= qui doivent être réimplémentées par les classes enfants)
	class SEL_ENGINE_API Renderable // interface
	{
		public:
			// Il est important pour une classe virtuelle de base d'avoir un destructeur virtuel
			// Cela permet de s'assurer que le destructeur enfant est bien appelé
			virtual ~Renderable() = default;

			virtual void Draw(Renderer& renderer, const Matrix3f& matrix) const = 0;

			virtual SDL_FRect GetBounds() const = 0;

			virtual void PopulateInspector(WorldEditor& worldEditor) = 0;
			virtual nlohmann::json Serialize() const = 0;
	};
}
