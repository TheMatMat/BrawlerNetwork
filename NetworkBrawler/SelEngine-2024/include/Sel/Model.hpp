#pragma once

#include <Sel/Asset.hpp>
#include <Sel/Color.hpp>
#include <Sel/Export.hpp>
#include <Sel/Renderable.hpp>
#include <Sel/Vector2.hpp>
#include <nlohmann/json_fwd.hpp> //< header spécial qui fait des déclarations anticipées des classes de la lib
#include <SDL.h>
#include <memory>
#include <string>
#include <vector>

namespace Sel
{
	class Renderer;
	class Texture;
	class Transform;
	class WorldEditor;

	struct ModelVertex
	{
		Vector2f pos;
		Vector2f uv;
		Color color;
	};

	class SEL_ENGINE_API Model : public Asset, public Renderable // Un ensemble de triangles
	{
		public:
			Model();
			Model(std::shared_ptr<Texture> texture, std::vector<ModelVertex> vertices, std::vector<int> indices, std::string filepath);
			Model(const Model&) = delete;
			Model(Model&&) = default;
			~Model() = default;

			void Draw(Renderer& renderer, const Matrix3f& matrix) const override;

			SDL_FRect GetBounds() const override;
			const std::vector<ModelVertex>& GetVertices() const;

			bool IsValid() const;

			void PopulateInspector(WorldEditor& worldEditor) override;

			bool SaveToFile(const std::string& filepath) const;
			nlohmann::ordered_json SaveToJSon() const;

			nlohmann::json Serialize() const override;

			Model& operator=(const Model&) = delete;
			Model& operator=(Model&&) = default;

			static Model LoadFromFile(const std::string& filepath);
			static Model LoadFromJSon(const nlohmann::json& doc, std::string filepath = {});
			static std::shared_ptr<Model> Unserialize(const nlohmann::json& doc);

		private:
			bool SaveToFileRegular(const std::string& filepath) const;
			bool SaveToFileCompressed(const std::string& filepath) const;
			bool SaveToFileBinary(const std::string& filepath) const;

			static Model LoadFromFileRegular(const std::string& filepath);
			static Model LoadFromFileCompressed(const std::string& filepath);
			static Model LoadFromFileBinary(const std::string& filepath);

			std::shared_ptr<Texture> m_texture;
			std::vector<ModelVertex> m_vertices;
			// Tableau de sommets calculés à chaque Draw et passé à SDL_RenderGeometry, en membre pour éviter d'allouer un nouveau tableau à chaque frame
			// mutable permet de modifier ("muter") le membre même dans une méthode constante, ce qui est très utile pour un cache comme sdlVertices
			mutable std::vector<SDL_Vertex> m_sdlVertices;
			std::vector<int> m_indices;
			SDL_FRect m_bounds;
	};
}