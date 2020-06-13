#ifndef TEST_MOVE_SYSTEM_H
#define TEST_MOVE_SYSTEM_H

#include <apollo/system.h>
#include <apollo/core/common.h>

class move_system : public apollo::system
{
public:
	void update(apollo::registry* registry) override
	{
		registry->for_each([](apollo::entity& entity, transform& t) {
			t.m_x++;
		});
	}
};

#endif // !TEST_MOVE_SYSTEM_H
