#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

// Ce fichier contient des constantes pouvant être utiles à la fois côté serveur et client

const std::uint16_t AppPort = 14769;

// Taille de base des éléments
constexpr float BoxSize = 64.f;
constexpr float CircleRadius = 32.f;
constexpr float SegmentLength = 128.f;

// Tickrate physique et réseau
constexpr float TickDelay = 1.f / 30.f;

// Taille maximale du nom d'un joueur
constexpr std::size_t MaxPlayerNameLength = 16;