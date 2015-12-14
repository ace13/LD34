#include "ParticleManager.hpp"

#include <Core/Math.hpp>
#include <SFML/Graphics/VertexArray.hpp>

ParticleManager::ParticleManager()
{
	mParticles.reserve(MAX_PARTICLES);
}

ParticleManager::~ParticleManager()
{

}

void ParticleManager::addParticle(const Particle& particle, const sf::Vector2f& pos, const sf::Vector2f& dir, float angle)
{
	if (mParticles.size() < MAX_PARTICLES)
		mParticles.push_back({ &particle, Timespan(0), pos, dir, angle });
}

void ParticleManager::update(const Timespan& span)
{
	float dt = Time::Seconds(span);
	for (auto it = mParticles.begin(); it != mParticles.end();)
	{
		it->LifeTime += span;

		it->Angle += it->Definition->Rotation * dt;
		it->Velocity += it->Definition->Gravity * dt;
		if (it->Definition->Friction != 0)
			it->Velocity *= it->Definition->Friction;

		it->Position += it->Velocity * dt;
		if (it->LifeTime > it->Definition->LifeTime)
			it = mParticles.erase(it);
		else
			++it;
	}
}

namespace
{
	sf::Color lerpColor(const sf::Color& start, const sf::Color& end, float t)
	{
		return{
			uint8_t(float(start.r) + (float(end.r) - start.r) * t),
			uint8_t(float(start.g) + (float(end.g) - start.g) * t),
			uint8_t(float(start.b) + (float(end.b) - start.b) * t),
			uint8_t(float(start.a) + (float(end.a) - start.a) * t)
		};
	}
}

void ParticleManager::draw(sf::RenderTarget& target)
{
	sf::VertexArray arr(sf::Quads, mParticles.size() * 4);

	int i = 0;
	for (auto& it : mParticles)
	{
		float life = Time::Seconds(it.LifeTime) / Time::Seconds(it.Definition->LifeTime);
		auto& pos = it.Position;
		auto& s = it.Definition->Size;
		auto col = lerpColor(it.Definition->StartColor, it.Definition->EndColor, life);

		const sf::Vector2f Left{ cos(it.Angle), sin(it.Angle) };
		const sf::Vector2f Top{ cos(it.Angle - Math::PI2), sin(it.Angle - Math::PI2) };

		arr[i++] = {
			it.Position - Top * s.y - Left * s.x,
			col
		};
		arr[i++] = {
			it.Position - Top * s.y + Left * s.x,
			col
		};
		arr[i++] = {
			it.Position + Top * s.y + Left * s.x,
			col
		};
		arr[i++] = {
			it.Position + Top * s.y - Left * s.x,
			col
		};
	}

	target.draw(arr);
}

void ParticleManager::clear()
{
	mParticles.clear();
}