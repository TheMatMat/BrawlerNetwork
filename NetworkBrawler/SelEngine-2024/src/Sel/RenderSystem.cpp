#include <Sel/RenderSystem.hpp>
#include <Sel/CameraComponent.hpp>
#include <Sel/GraphicsComponent.hpp>
#include <Sel/Renderable.hpp>
#include <Sel/Transform.hpp>
#include <fmt/color.h>
#include <fmt/core.h>
#include <entt/entt.hpp>

namespace Sel
{
	RenderSystem::RenderSystem(Renderer& renderer, entt::registry& registry) :
	m_renderer(renderer),
	m_registry(registry)
	{
	}

	void RenderSystem::Update(float /*deltaTime*/)
	{
		// Sélection de la caméra
		Matrix3f cameraMatrix = Matrix3f::Identity();

		auto cameraView = m_registry.view<Transform, CameraComponent>();
		bool cameraFound = false;
		for (entt::entity entity : cameraView)
		{
			// Nous avons déjà une caméra ?
			if (cameraFound)
			{
				fmt::print(stderr, fg(fmt::color::red), "warning: multiple camera found\n");
				break; // On s'arrête là, pas besoin d'afficher le warning plus d'une fois par frame
			}
			
			// La matrice de vue (celle de la caméra) est une matrice de transformation inversée
			// En effet, décaler la caméra à gauche revient à déplacer le monde entier à droite, etc.
			Transform& entityTransform = cameraView.get<Transform>(entity);
			cameraMatrix = entityTransform.GetTransformMatrix();
			cameraMatrix = cameraMatrix.GetInverse();
			cameraFound = true;
		}

		// Si pas de caméra trouvée on affiche un warning (mais la transformation reste à l'identité)
		if (!cameraFound)
			fmt::print(stderr, fg(fmt::color::red), "warning: no camera found\n");

		auto view = m_registry.view<Transform, GraphicsComponent>();
		for (entt::entity entity : view)
		{
			Transform& entityTransform = view.get<Transform>(entity);
			GraphicsComponent& entityGraphics = view.get<GraphicsComponent>(entity);
			if (!entityGraphics.renderable)
				continue;

			// Construction de la matrice de transformation de l'entité
			// Matrice "monde" (aussi appelée modèle), passage du repère local au repère monde
			Matrix3f worldMatrix = entityTransform.GetTransformMatrix();

			// On y applique ensuite la matrice de vue (matrice de transformation de la caméra inversée)
			Matrix3f worldViewMatrix = cameraMatrix * worldMatrix;

			// Et on affiche l'entité via son interface Renderable
			entityGraphics.renderable->Draw(m_renderer, worldViewMatrix);
		}
	}	
}