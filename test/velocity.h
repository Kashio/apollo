#ifndef TEST_VELOCITY_H
#define TEST_VELOCITY_H

#include <apollo/component.h>

struct velocity : public apollo::component<velocity>
{
	float m_velocity;

	velocity() = default;

	velocity(float velocity)
		: m_velocity(velocity)
	{}
};

#endif // !TEST_VELOCITY_H
