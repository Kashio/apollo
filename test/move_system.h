#ifndef TEST_MOVE_SYSTEM_H
#define TEST_MOVE_SYSTEM_H

#include <apollo/system.h>

class move_system : public apollo::system
{
public:
	void update(apollo::registry* registry) override
	{
		registry->for_each([](transform& t, mass& m) {
			t.m_x++;
			m.m_mass++;
		});
	}
};

#endif // !TEST_MOVE_SYSTEM_H
