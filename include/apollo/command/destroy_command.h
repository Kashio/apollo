#ifndef APOLLO_COMMAND_DESTROY_COMMAND
#define APOLLO_COMMAND_DESTROY_COMMAND

#include "command.h"
#include "../core/common.h"

namespace apollo
{
	class destroy_command : public command
	{
	private:
		entity const & m_entity;
	public:
		destroy_command(registry* registry, entity const & entity)
			: command(registry)
			, m_entity(entity)
		{
		}

		void execute() override
		{
			m_registry->destroy(m_entity);
		}
	};
}

#endif // !APOLLO_COMMAND_DESTROY_COMMAND
