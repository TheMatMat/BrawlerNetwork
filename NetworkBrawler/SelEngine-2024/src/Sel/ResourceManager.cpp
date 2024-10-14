#include <Sel/ResourceManager.hpp>
#include <Sel/Font.hpp>
#include <Sel/Model.hpp>
#include <Sel/Spritesheet.hpp>
#include <Sel/Surface.hpp>
#include <Sel/Texture.hpp>
#include <fmt/color.h>
#include <fmt/core.h>
#include <stdexcept>

namespace Sel
{
	ResourceManager::ResourceManager(Renderer& renderer) :
	m_renderer(renderer)
	{
		if (s_instance != nullptr)
			throw std::runtime_error("only one ResourceManager can be created");

		s_instance = this;
	}

	ResourceManager::~ResourceManager()
	{
		s_instance = nullptr;
	}

	void ResourceManager::Clear()
	{
		m_missingModel.reset();
		m_missingSpritesheet.reset();
		m_missingTexture.reset();
		m_textures.clear();
	}

	const std::shared_ptr<Font>& ResourceManager::GetFont(const std::string& fontPath)
	{
		// Woop, woop, Avons-nous déjà cette police en stock ?
		auto it = m_fonts.find(fontPath);
		if (it != m_fonts.end())
			return it->second; // Oui, on peut le renvoyer

		// Non, essayons de le charger
		Font font = Font::OpenFromFile(fontPath);
		// Si nous arrivons ici c'est que la police est ouverte (sinon une exception aurait été lancée)

		it = m_fonts.emplace(fontPath, std::make_shared<Font>(std::move(font))).first;
		return it->second;
	}

	const std::shared_ptr<Model>& ResourceManager::GetModel(const std::string& modelPath)
	{
		// Avons-nous déjà ce modèle en stock ?
		auto it = m_models.find(modelPath);
		if (it != m_models.end())
			return it->second; // Oui, on peut le renvoyer

		// Non, essayons de le charger
		Model model = Model::LoadFromFile(modelPath);
		if (!model.IsValid())
		{
			// On a pas pu charger le modèle, utilisons un modèle "manquant"
			if (!m_missingModel)
				m_missingModel = std::make_shared<Model>();

			m_models.emplace(modelPath, m_missingModel);
			return m_missingModel;
		}

		it = m_models.emplace(modelPath, std::make_shared<Model>(std::move(model))).first;
		return it->second;
	}

	const std::shared_ptr<Spritesheet>& ResourceManager::GetSpritesheet(const std::string& spritesheetPath)
	{
		// Même logique que GetTexture
		auto it = m_spritesheets.find(spritesheetPath);
		if (it != m_spritesheets.end())
			return it->second;

		try
		{
			std::shared_ptr<Spritesheet> spritesheet = std::make_shared<Spritesheet>(Spritesheet::LoadFromFile(spritesheetPath));

			it = m_spritesheets.emplace(spritesheetPath, std::move(spritesheet)).first;
			return it->second;
		}
		catch (const std::exception& e)
		{
			fmt::print(fg(fmt::color::red), "failed to load {0}: {1}", spritesheetPath, e.what());

			// Difficile de renvoyer une spritesheet autre que vide
			if (!m_missingSpritesheet)
				m_missingSpritesheet = std::make_shared<Spritesheet>();

			m_spritesheets.emplace(spritesheetPath, m_missingSpritesheet);
			return m_missingSpritesheet;
		}
	}

	const std::shared_ptr<Texture>& ResourceManager::GetTexture(const std::string& texturePath)
	{
		// Avons-nous déjà cette texture en stock ?
		auto it = m_textures.find(texturePath);
		if (it != m_textures.end())
			return it->second; // Oui, on peut la renvoyer

		// Non, essayons de la charger
		try
		{
			// Texture::LoadFromFile renvoie une exception si elle n'arrive pas à charger le fichier
			// d'où le bloc try/catch car nous avons un fallback (bonus)
			// Note: il aurait été aussi possible de renvoyer un objet vide et de le tester avec une méthode comme IsValid
			std::shared_ptr<Texture> texture = std::make_shared<Texture>(Texture::LoadFromFile(m_renderer, texturePath));

			// .emplace et .insert renvoient un std::pair<iterator, bool>, le booléen indiquant si la texture a été insérée dans la map (ce qu'on sait déjà ici)
			it = m_textures.emplace(texturePath, std::move(texture)).first;

			// Attention, on ne peut pas renvoyer texture directement (même sans std::move) car on renvoie une référence constante
			// qui serait alors une référence constante sur une variable temporaire détruite à la fin de la fonction (texture)

			return it->second;
		}
		catch (const std::exception& e)
		{
			fmt::print(fg(fmt::color::red), "failed to load {0}: {1}", texturePath, e.what());

			// Le chargement de la surface (ou la création de sa texture) a échoué, renvoyons une texture "manquante"
			if (!m_missingTexture)
			{
				// On créé la texture la première fois qu'on en a besoin
				Surface missingNo = Surface::Create(64, 64);
				missingNo.FillRect(SDL_Rect{ 0, 0, 16, 16 }, 255, 0, 255, 255);
				missingNo.FillRect(SDL_Rect{ 16, 0, 16, 16 }, 0, 0, 0, 255);
				missingNo.FillRect(SDL_Rect{ 0, 16, 16, 16 }, 0, 0, 0, 255);
				missingNo.FillRect(SDL_Rect{ 16, 16, 16, 16 }, 255, 0, 255, 255);

				m_missingTexture = std::make_shared<Texture>(Texture::CreateFromSurface(m_renderer, missingNo));
			}

			// On enregistre cette texture comme une texture manquante (pour ne pas essayer de la charger à chaque fois)
			m_textures.emplace(texturePath, m_missingTexture);
			return m_missingTexture;
		}
	}

	void ResourceManager::Purge()
	{
		// On va itérer sur le conteneur tout en enlevant certains éléments pendant l'itération, cela demande un peu de pratique
		for (auto it = m_textures.begin(); it != m_textures.end(); ) //< pas d'incrémentation de it
		{
			// On vérifie le compteur pour vérifier si la texture est utilisée ailleurs ou non
			if (it->second.use_count() > 1)
			{
				++it; // la texture est utilisée, on la garde et on passe à la suivante
			}
			else
			{
				// la texture n'est plus utilisée, on peut l'enlever avec .erase(it), qui renvoie un nouvel itérateur sur l'élément *suivant*
				// (celui du prochain tour de boucle = pas d'incrémentation dans ce cas)
				it = m_textures.erase(it);
			}
		}
	}

	ResourceManager& ResourceManager::Instance()
	{
		if (s_instance == nullptr)
			throw std::runtime_error("ResourceManager hasn't been instantied");

		return *s_instance; 
	}

	ResourceManager* ResourceManager::s_instance = nullptr;
}