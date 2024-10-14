#include <Sel/Model.hpp>
#include <Sel/ResourceManager.hpp>
#include <Sel/Renderer.hpp>
#include <Sel/BinarySerializer.hpp>
#include <Sel/Texture.hpp>
#include <Sel/Transform.hpp>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/std.h>
#include <imgui.h>
#include <lz4.h>
#include <nlohmann/json.hpp>
#include <cassert>
#include <fstream>

namespace Sel
{
	constexpr unsigned int FileVersion = 1;

	Model::Model() :
	Asset("")
	{
	}

	Model::Model(std::shared_ptr<Texture> texture, std::vector<ModelVertex> vertices, std::vector<int> indices, std::string filepath) :
	Asset(std::move(filepath)),
	m_texture(std::move(texture)),
	m_vertices(std::move(vertices)),
	m_indices(std::move(indices))
	{
		/*
		Pour afficher notre modèle, nous devons donner à la SDL un vector<SDL_Vertex>, nous aurions pu directement en prendre un (au lieu d'un vector<ModelVertex>) mais nous aurions été 
		confronté au même problème : lors de l'affichage, pour appliquer le Transform, nous devons calculer la nouvelle position.
		Pour affecter la transformation aux vertices (sans perdre leur valeur originale) nous devons dupliquer l'information, avec un autre std::vector<SDL_Vertex>.
		Néanmoins, faire un nouveau vector à chaque affichage (= opération très fréquente) signifierait faire une grosse allocation mémoire très souvent, ce qui est généralement à éviter.
		On peut ici être astucieux et conserver le vector en membre, d'une exécution à l'autre et réutiliser sa mémoire.

		De plus, comme tex_coord et color ne sont pas affectés par le Transform, on peut les précalculer à la construction directement
		*/

		Vector2f maxs(std::numeric_limits<float>::min()); // -Infinity
		Vector2f mins(std::numeric_limits<float>::max()); // +Infinity

		m_sdlVertices.resize(m_vertices.size());
		for (std::size_t i = 0; i < m_vertices.size(); ++i)
		{
			const ModelVertex& modelVertex = m_vertices[i];
			SDL_Vertex& sdlVertex = m_sdlVertices[i];

			// Calcul des coordonnées minimales et maximales
			maxs.x = std::max(maxs.x, modelVertex.pos.x);
			maxs.y = std::max(maxs.y, modelVertex.pos.y);
			mins.x = std::min(mins.x, modelVertex.pos.x);
			mins.y = std::min(mins.x, modelVertex.pos.y);

			// Conversion de nos structures vers les structures de la SDL
			sdlVertex.tex_coord = SDL_FPoint{ modelVertex.uv.x, modelVertex.uv.y };

			std::uint8_t r, g, b, a;
			modelVertex.color.ToRGBA8(r, g, b, a);

			sdlVertex.color = SDL_Color{ r, g, b, a };
		}

		// Maintenant que nous avons les coordonnées minimales et maximales nous pouvons facilement définir le rectangle
		m_bounds.x = mins.x;
		m_bounds.y = mins.y;
		m_bounds.w = maxs.x - mins.x; // longueur = max X moins min X
		m_bounds.h = maxs.y - mins.y;
	}

	void Model::Draw(Renderer& renderer, const Matrix3f& matrix) const
	{
		// On s'assure que les deux tableaux font la même taille (assert crash immédiatement le programme si la condition passée est fausse)
		assert(m_vertices.size() == m_sdlVertices.size());
		for (std::size_t i = 0; i < m_vertices.size(); ++i)
		{
			const ModelVertex& modelVertex = m_vertices[i];
			SDL_Vertex& sdlVertex = m_sdlVertices[i];

			// tex_coord et color sont déjà gérés par le constructeur
			Vector2f transformedPos = matrix * modelVertex.pos;
			sdlVertex.position = SDL_FPoint{ transformedPos.x, transformedPos.y };
		}

		if (!m_indices.empty())
		{
			SDL_RenderGeometry(renderer.GetHandle(),
				(m_texture) ? m_texture->GetHandle() : nullptr,
				m_sdlVertices.data(), static_cast<int>(m_sdlVertices.size()),
				m_indices.data(), static_cast<int>(m_indices.size()));
		}
		else
		{
			SDL_RenderGeometry(renderer.GetHandle(),
				(m_texture) ? m_texture->GetHandle() : nullptr,
				m_sdlVertices.data(), static_cast<int>(m_sdlVertices.size()),
				nullptr, 0);
		}
	}

	SDL_FRect Model::GetBounds() const
	{
		return m_bounds;
	}

	const std::vector<ModelVertex>& Model::GetVertices() const
	{
		return m_vertices;
	}

	bool Model::IsValid() const
	{
		// Un modèle peut ne pas avoir de texture/indices, mais il a forcément des vertices
		return !m_vertices.empty();
	}

	void Model::PopulateInspector(WorldEditor& /*worldEditor*/)
	{
	}

	bool Model::SaveToFile(const std::string& filepath) const
	{
		if (filepath.ends_with(".model"))
			return SaveToFileRegular(filepath);
		else if (filepath.ends_with(".cmodel"))
			return SaveToFileCompressed(filepath);
		else if (filepath.ends_with(".bmodel"))
			return SaveToFileBinary(filepath);
		else
		{
			fmt::print(stderr, fg(fmt::color::red), "unknown extension {}\n", filepath.substr(filepath.find_last_of(".")));
			return false;
		}
	}

	nlohmann::ordered_json Model::SaveToJSon() const
	{
		// nlohmann::json et nlohmann::ordered_json ont les mêmes fonctionnalités, mais ce dernier préserve l'ordre d'insertion des clés (qui ne change rien aux données, c'est juste plus joli :D )

		// On va sauvegarder tous les champs intéressants à l'aide de l'opérateur [] de nlohmann::json qui, comme pour std::(unordered_)map, créé une clé/valeur si la clé n'existe pas
		// une autre approche aurait été d'instancier les documents puis de les affecter
		nlohmann::ordered_json doc;

		// L'ajout d'un champ "version" permet de faire évoluer le format dans le futur
		doc["version"] = FileVersion;

		// Faisons référence à la texture via son chemin, si elle en a un
		if (m_texture)
		{
			const std::string& texturePath = m_texture->GetFilepath();
			if (!texturePath.empty())
				doc["texture"] = texturePath;
		}

		// On enregistre les indices si nous en avons
		if (!m_indices.empty())
			doc["indices"] = m_indices;

		nlohmann::ordered_json& vertices = doc["vertices"];
		for (const ModelVertex& modelVertex : m_vertices)
		{
			nlohmann::ordered_json& vertex = vertices.emplace_back();
			
			nlohmann::ordered_json& pos = vertex["pos"];
			pos["x"] = modelVertex.pos.x;
			pos["y"] = modelVertex.pos.y;

			nlohmann::ordered_json& uv = vertex["uv"];
			uv["u"] = modelVertex.uv.x;
			uv["v"] = modelVertex.uv.y;

			nlohmann::ordered_json& color = vertex["color"];
			color["r"] = modelVertex.color.r;
			color["g"] = modelVertex.color.g;
			color["b"] = modelVertex.color.b;

			// Le champ "a" (alpha) est optionnel et sa valeur sera de 1 si on ne l'enregistre pas
			// Cela permet d'économiser un peu de place
			if (modelVertex.color.a != 1.f)
				color["a"] = modelVertex.color.a;
		}

		return doc;
	}

	nlohmann::json Model::Serialize() const
	{
		nlohmann::json renderableDoc;
		renderableDoc["Type"] = "Model";

		// On sauvegarde le chemin vers le modèle si possible, sinon le modèle entier
		if (const std::string& filepath = GetFilepath(); !filepath.empty())
			renderableDoc["Path"] = filepath;
		else
			renderableDoc["Model"] = SaveToJSon();

		return renderableDoc;
	}

	Model Model::LoadFromFile(const std::string& filepath)
	{
		if (filepath.ends_with(".model"))
			return LoadFromFileRegular(filepath);
		else if (filepath.ends_with(".cmodel"))
			return LoadFromFileCompressed(filepath);
		else if (filepath.ends_with(".bmodel"))
			return LoadFromFileBinary(filepath);
		else
		{
			fmt::print(stderr, fg(fmt::color::red), "unknown extension {}\n", filepath.substr(filepath.find_last_of(".")));
			return {};
		}
	}

	Model Model::LoadFromJSon(const nlohmann::json& doc, std::string filepath)
	{
		// Le champ version nous permet de savoir si le format a été généré par une version ultérieure de notre programme
		// qui serait incompatible avec notre propre version
		unsigned int version = doc["version"];
		if (version > FileVersion)
		{
			fmt::print(stderr, fg(fmt::color::red), "model file has unsupported version {} (current version is {})", version, FileVersion);
			return {}; //< on retourne un Model construit par défaut (on pourrait également lancer une exception)
		}

		std::string texturePath = doc.value("texture", "");

		// Textures
		std::shared_ptr<Texture> texture;
		if (!texturePath.empty())
			texture = ResourceManager::Instance().GetTexture(texturePath);

		// Indices
		std::vector<int> indices;
		
		// Astuce: en C++ l'écriture `if (init; cond)` est autorisée et permet d'initialiser une variable temporaire
		// mais de spécifier la façon dont on va faire le test (la condition du if est donc ici `it != doc.end()`)
		if (auto it = doc.find("indices"); it != doc.end())
		{
			const nlohmann::json& indiceArray = it.value();
			for (int i : indiceArray)
				indices.push_back(i);
		}

		// Vertices
		const nlohmann::json& verticeArray = doc["vertices"];

		std::vector<ModelVertex> vertices;
		for (const nlohmann::json& vertex : verticeArray)
		{
			// emplace_back construit un élément dans un vector et retourne une référence sur celui-ci
			ModelVertex& modelVertex = vertices.emplace_back();

			const nlohmann::json& positionDoc = vertex["pos"];
			modelVertex.pos = Vector2f(positionDoc["x"], positionDoc["y"]);

			if (auto it = vertex.find("uv"); it != vertex.end())
			{
				const nlohmann::json& uvDoc = it.value();
				modelVertex.uv = Vector2f(uvDoc["u"], uvDoc["v"]);
			}

			if (auto it = vertex.find("color"); it != vertex.end())
			{
				const nlohmann::json& colorDoc = it.value();
				modelVertex.color = Color(colorDoc["r"], colorDoc["g"], colorDoc["b"], colorDoc.value("a", 1.f));
			}
		}

		return Model(std::move(texture), std::move(vertices), std::move(indices), std::move(filepath));
	}

	std::shared_ptr<Model> Model::Unserialize(const nlohmann::json& doc)
	{
		std::string filepath = doc.value("Path", "");
		if (!filepath.empty())
			return ResourceManager::Instance().GetModel(filepath);
		else
			return std::make_shared<Model>(LoadFromJSon(doc["Model"]));
	}

	bool Model::SaveToFileRegular(const std::string& filepath) const
	{
		// Ouverture d'un fichier en écriture
		std::ofstream outputFile(filepath);
		if (!outputFile.is_open())
		{
			fmt::print(stderr, fg(fmt::color::red), "failed to open model file {}\n", filepath);
			return false;
		}

		nlohmann::ordered_json doc = SaveToJSon();
		outputFile << doc.dump(4);

		// Pas besoin de fermer le fichier, le destructeur de std::ofstream s'en occupe (c'est bon les destructeurs, mangez-en !)
		return true;
	}

	bool Model::SaveToFileCompressed(const std::string& filepath) const
	{
		// Ouverture d'un fichier en écriture (et en mode binaire car nous ne stockons pas du texte)
		std::ofstream outputFile(filepath, std::ios::binary);
		if (!outputFile.is_open())
		{
			fmt::print(stderr, fg(fmt::color::red), "failed to open model file {}\n", filepath);
			return false;
		}

		nlohmann::ordered_json doc = SaveToJSon();

		std::string jsonStr = doc.dump();

		// Nous devons allouer un tableau d'une taille suffisante pour stocker la version compressée
		// La fonction LZ4_compressBound nous donne la taille compressée maximale possible avec une taille donnée
		int maxSize = LZ4_compressBound(static_cast<int>(jsonStr.size()));

		// Lors de la décompression en revanche, nous n'avons pas d'équivalent pour calculer la taille maximale décompressée
		// nous allons donc stocker la taille au début du fichier, en binaire.
		// note: il est important d'utiliser un type à taille fixe pour que ce soit lisible sur plusieurs machines
		// il faudrait également prendre l'endianness en compte mais cela dépasse le cadre de cet exercice
		std::uint32_t decompressedSize = static_cast<std::uint32_t>(jsonStr.size());
		outputFile.write(reinterpret_cast<const char*>(&decompressedSize), sizeof(std::uint32_t));

		// Nous pouvons ensuite allouer un tableau d'octets (char), std::vector<char> ferait l'affaire ici mais un unique_ptr suffit amplement
		std::unique_ptr<char[]> compressedStr = std::make_unique<char[]>(maxSize);

		// On peut ensuite compresser avec la fonction LZ4_compress_default
		// celle-ci prend :
		// 1) une suite d'octets à compresser en entrée, 
		// 2) un pointeur vers l'endroit où stocker le résultat
		// 3) ainsi que la taille des données à compresser
		// 4) la taille du buffer final, par sécurité
		int finalSize = LZ4_compress_default(jsonStr.data(), compressedStr.get(), static_cast<int>(jsonStr.size()), static_cast<int>(maxSize));
		if (finalSize == 0)
		{
			// La compression a échoué (ça ne devrait pas arriver)
			fmt::print(stderr, fg(fmt::color::red), "failed to compress model file\n");
			return false;
		}

		// En cas de succès, LZ4_compress_default renvoie la taille finale des données compressées, nous pouvons alors l'écrire directement
		// en mode binaire on utilisera alors plutôt .write
		outputFile.write(compressedStr.get(), finalSize);

		return true;
	}

	bool Model::SaveToFileBinary(const std::string& filepath) const
	{
		// Ouverture d'un fichier en écriture (et en mode binaire car nous ne stockons pas du texte)
		std::ofstream outputFile(filepath, std::ios::binary);
		if (!outputFile.is_open())
		{
			fmt::print(stderr, fg(fmt::color::red), "failed to open model file {}\n", filepath);
			return false;
		}

		// Il est important d'utiliser un type à taille fixe pour que ce soit lisible sur plusieurs machines
		// Il faudrait également prendre l'endianness en compte mais cela dépasse le cadre de cet exercice
		std::vector<std::uint8_t> byteArray;
		SerializeBinary(byteArray, static_cast<std::uint8_t>(FileVersion));

		// Texture (chemin)
		SerializeBinary(byteArray, m_texture->GetFilepath());

		// Indices (nombre indices + indices)
		SerializeBinary(byteArray, static_cast<std::uint32_t>(m_indices.size()));

		for (int index : m_indices)
			SerializeBinary(byteArray, static_cast<std::int32_t>(index));

		// Vertices (nombre vertices + vertices)
		SerializeBinary(byteArray, static_cast<std::uint32_t>(m_vertices.size()));

		for (const auto& vertex : m_vertices)
		{
			// float est, en pratique, un taille à type fixe
			SerializeBinary(byteArray, vertex.pos.x);
			SerializeBinary(byteArray, vertex.pos.y);
			SerializeBinary(byteArray, vertex.uv.x);
			SerializeBinary(byteArray, vertex.uv.y);
			SerializeBinary(byteArray, vertex.color.r);
			SerializeBinary(byteArray, vertex.color.g);
			SerializeBinary(byteArray, vertex.color.b);
			SerializeBinary(byteArray, vertex.color.a);
		}

		// On écrit le buffer dans le fichier
		outputFile.write(reinterpret_cast<const char*>(byteArray.data()), byteArray.size());

		return true;
	}

	Model Model::LoadFromFileRegular(const std::string& filepath)
	{
		// Ouverture d'un fichier en lecture
		std::ifstream inputFile(filepath);
		if (!inputFile.is_open())
		{
			fmt::print(stderr, fg(fmt::color::red), "failed to open model file {}\n", filepath);
			return {}; //< on retourne un Model construit par défaut (on pourrait également lancer une exception)
		}

		return LoadFromJSon(nlohmann::json::parse(inputFile), filepath);
	}

	Model Model::LoadFromFileCompressed(const std::string& filepath)
	{
		// Ouverture d'un fichier en lecture (en binaire)
		std::ifstream inputFile(filepath, std::ios::binary);
		if (!inputFile.is_open())
		{
			fmt::print(stderr, fg(fmt::color::red), "failed to open model file {}\n", filepath);
			return {}; //< on retourne un Model construit par défaut (on pourrait également lancer une exception)
		}

		// On lit tout le contenu dans un vector
		std::vector<char> content((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());

		// Nous devons allouer un tableau d'une taille suffisante pour stocker la version décompressée : problème, nous n'avons pas cette information
		// Nous l'avons donc stockée dans un std::uint32_t au début du fichier
		std::uint32_t decompressedSize;
		std::memcpy(&decompressedSize, &content[0], sizeof(std::uint32_t));

		// Petite sécurité pour éviter les données malveillantes : assurons-nous que la taille n'a pas une valeur délirante
		if (decompressedSize > 10'000'000) //< La taille dépasse 10MB? Évitons
		{
			fmt::print(stderr, fg(fmt::color::red), "failed to load model file {}: decompressed size is too big ({}), is the file corrupt?\n", filepath, decompressedSize);
			return {};
		}

		// Nous pouvons ensuite allouer un tableau d'octets (char), std::vector<char> ferait l'affaire ici mais un unique_ptr suffit amplement
		std::unique_ptr<char[]> decompressedStr = std::make_unique<char[]>(decompressedSize);
		if (LZ4_decompress_safe(&content[sizeof(std::uint32_t)], decompressedStr.get(), static_cast<int>(content.size() - sizeof(std::uint32_t)), decompressedSize) <= 0)
		{
			fmt::print(stderr, fg(fmt::color::red), "failed to load model file {}: corrupt file\n", filepath);
			return {};
		}

		return LoadFromJSon(nlohmann::json::parse(decompressedStr.get()), filepath);
	}

	Model Model::LoadFromFileBinary(const std::string& filepath)
	{
		// Ouverture d'un fichier en lecture (en binaire)
		std::ifstream inputFile(filepath, std::ios::binary);
		if (!inputFile.is_open())
		{
			fmt::print(stderr, fg(fmt::color::red), "failed to open model file {}\n", filepath);
			return {}; //< on retourne un Model construit par défaut (on pourrait également lancer une exception)
		}

		// Normalement on devrait lire le fichier morceau par morceau plutôt que de le charger intégralement en mémoire (imaginez le chargement d'un fichier de plusieurs Go)
		// mais dans le cadre de cet exercice c'est bien suffisant
		std::vector<std::uint8_t> byteArray;

		// Une façon classique de récupérer la taille du fichier est de positionner le curseur de lecture à la fin ("0 par rapport à la fin") puis de récupérer sa position par rapport au début
		inputFile.seekg(0, std::ios::end);
		std::size_t length = inputFile.tellg();
		inputFile.seekg(0, std::ios::beg);

		byteArray.resize(length);
		inputFile.read(reinterpret_cast<char*>(byteArray.data()), length);

		std::size_t offset = 0;
		std::uint8_t version = UnserializeBinary<std::uint8_t>(byteArray, offset);
		if (version > FileVersion)
		{
			fmt::print(stderr, fg(fmt::color::red), "model file has unsupported version {} (current version is {})", version, FileVersion);
			return {}; //< on retourne un Model construit par défaut (on pourrait également lancer une exception)
		}

		Model model;

		// Texture (chemin)
		std::string texturePath = UnserializeBinary<std::string>(byteArray, offset);

		std::shared_ptr<Texture> texture;
		if (!texturePath.empty())
			texture = ResourceManager::Instance().GetTexture(texturePath);

		// Indices (nombre indices + indices)
		std::uint32_t indexCount = UnserializeBinary<std::uint32_t>(byteArray, offset);

		std::vector<int> indices;
		for (std::uint32_t i = 0; i < indexCount; ++i)
		{
			std::int32_t value = UnserializeBinary<std::int32_t>(byteArray, offset);
			indices.push_back(static_cast<int>(value));
		}

		// Vertices (nombre vertices + vertices)
		std::uint32_t vertexCount = UnserializeBinary<std::uint32_t>(byteArray, offset);

		std::vector<ModelVertex> vertices(vertexCount);
		for (auto& vertex : vertices)
		{
			// float est, en pratique, un taille à type fixe
			vertex.pos.x = UnserializeBinary<float>(byteArray, offset);
			vertex.pos.y = UnserializeBinary<float>(byteArray, offset);
			vertex.uv.x = UnserializeBinary<float>(byteArray, offset);
			vertex.uv.y = UnserializeBinary<float>(byteArray, offset);
			vertex.color.r = UnserializeBinary<float>(byteArray, offset);
			vertex.color.g = UnserializeBinary<float>(byteArray, offset);
			vertex.color.b = UnserializeBinary<float>(byteArray, offset);
			vertex.color.a = UnserializeBinary<float>(byteArray, offset);
		}

		return Model(std::move(texture), std::move(vertices), std::move(indices), filepath);
	}
}