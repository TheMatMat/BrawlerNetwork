#pragma once

#include <Sel/Export.hpp>
#include <SDL.h>
#include <functional> //< std::function
#include <string> //< std::string
#include <unordered_map> //< std::unordered_map est plus efficace que std::map pour une association clé/valeur

namespace Sel
{
	enum class MouseButton
	{
		Left,
		Right,
		Middle,
		X1,
		X2
	};

	class SEL_ENGINE_API InputManager
	{
		public:
			InputManager();
			InputManager(const InputManager&) = delete;
			InputManager(InputManager&&) = delete;
			~InputManager();

			// Lorsque l'action "action" se déclenche, on appellera "func"
			void BindAction(std::string action, std::function<void(bool /*active*/)> func);

			// Appuyer sur la touche "keyCode" déclenchera "action"
			void BindKeyPressed(SDL_KeyCode keyCode, std::string action);

			// Appuyer sur le bouton "button" déclenchera "action"
			void BindMouseButtonPressed(MouseButton button, std::string action);

			// Appuyer sur le bouton "button" du contrôleur (manette) déclenchera "action"
			void BindControllerButton(SDL_GameControllerButton button, std::string action);

			// Réinitialise toutes les associations clavier/souris vers des actions
			void ClearBindings();

			// Teste si une action est en cours
			bool IsActive(const std::string& action) const;

			// Gère l'événement de la SDL et déclenche les actions associées, s'il y en a
			void HandleEvent(const SDL_Event& event);

			InputManager& operator=(const InputManager&) = delete;
			InputManager& operator=(InputManager&&) = delete;

			static InputManager& Instance();

		private:
			void TriggerAction(const std::string& action);
			void ReleaseAction(const std::string& action);

			struct ActionData
			{
				std::function<void(bool /*active*/)> callback;
				unsigned int count = 0;
			};

			std::unordered_map<int /*mouseButton*/, std::string /*action*/> m_mouseButtonToAction;
			std::unordered_map<SDL_GameControllerButton /*controllerButton*/, std::string /*action*/> m_controllerButtonToAction;
			std::unordered_map<SDL_Keycode /*key*/, std::string /*action*/> m_keyToAction;
			std::unordered_map<std::string /*action*/, ActionData> m_actions;

			static InputManager* s_instance;
	};
}