#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

// Ce fichier contient des constantes pouvant �tre utiles � la fois c�t� serveur et client

const std::uint16_t AppPort = 14769;

const int WINDOW_WIDTH = 850;
const int WINDOW_HEIGHT = 520;

const float SCREEN_MARGIN = 10.f;

const float WORLD_MIN_X = -500.f;
const float WORLD_MAX_X = 500.f;
const float WORLD_MIN_Y = -500.f;
const float WORLD_MAX_Y = 500.f;

enum class CollectibleType : std::uint8_t
{
	Carrot,
	GoldenCarrot,
};

enum class GameState : std::uint8_t
{
	Lobby,
	GameRunning,
	EndScreen
};

enum class PlayerMode : std::uint8_t
{
	Pending,
	Playing,
	Dead,
	Spectating
};

enum class BrawlerSkin : std::uint8_t
{
	Rabbit,
	Sheep,
	Turtle
};

struct OneShotAnimation
{
	bool isPlaying;
	std::string animationName;
	float animationDuration;
};

constexpr float StealAnimationDuration = 0.5f;

// Tickrate physique et r�seau
constexpr float TickDelay = 1.f / 30.f;

// Taille maximale du nom d'un joueur
constexpr std::size_t MaxPlayerNameLength = 16;