#pragma once

#include <Sel/Export.hpp>
#include <entt/fwd.hpp>
#include <nlohmann/json_fwd.hpp>

namespace Sel
{
	// Composant vide servant à tagger des entités
	// (on pourrait néanmoins rajouter des propriétés comme un ordre de priorité, 
	// un booléen disant si c'est actif ou non, ...)
	struct SEL_ENGINE_API CameraComponent
	{
	};
}
