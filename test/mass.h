#ifndef TEST_MASS_H
#define TEST_MASS_H

#include <apollo/component.h>

struct mass : public apollo::component<mass>
{
	float m_mass;

	mass() = default;

	mass(float mass)
		: m_mass(mass)
	{}
};

#endif // !TEST_MASS_H
