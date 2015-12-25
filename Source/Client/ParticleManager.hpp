#pragma once

#include "ResourceManager.hpp"

#include <Core/Time.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>

#include <vector>

class ParticleManager
{
public:
	struct Particle
	{
		Timespan LifeTime;

		sf::FloatRect TextureRect;
		sf::Color StartColor, EndColor;
		sf::Vector2f Gravity;
		float Friction, Rotation;
	};

	static const int MAX_PARTICLES = 2048;

	ParticleManager();
	~ParticleManager();

	void addParticle(const Particle& particle, const sf::Vector2f& pos, const sf::Vector2f& dir, float angle);
	void setTexture(const ResourceManager::Texture& name);

	void update(const Timespan& dt);
	void draw(sf::RenderTarget& target);

	void clear();

private:
	struct ParticleImpl
	{
		const Particle* Definition;

		Timespan LifeTime;
		sf::Vector2f Position, Velocity;
		float Angle;
	};

	std::vector<ParticleImpl> mParticles;
	ResourceManager::Texture mTexture;
};
