#ifndef TEST_TRANSFORM_H
#define TEST_TRANSFORM_H

#include <apollo/component.h>

struct transform : public apollo::component<transform>
{
	float m_x;
	float m_y;
	float m_z;

	transform() = default;

	transform(float x, float y, float z)
		: m_x(x), m_y(y), m_z(z)
	{}
};

#endif // !TEST_TRANSFORM_H
