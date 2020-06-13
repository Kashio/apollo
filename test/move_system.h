#ifndef TEST_MOVE_SYSTEM_H
#define TEST_MOVE_SYSTEM_H

#include <apollo/system.h>
#include <apollo/core/common.h>
#include <apollo/command/destroy_command.h>

class move_system : public apollo::system
{
public:
	void update(apollo::registry* registry) override
	{
		apollo::command_buffer cb = registry->create_command_buffer();
		registry->for_each([&cb](apollo::entity& entity, transform& t) {
			t.m_x++;
			cb.add_command<apollo::destroy_command>(entity);
		});
		cb.execute();
	}
};

#endif // !TEST_MOVE_SYSTEM_H
