#ifndef APOLLO_COMMAND_DESTROY_COMMAND
#define APOLLO_COMMAND_DESTROY_COMMAND

#include "command.h"
#include "../core/common.h"

namespace apollo
{
	template <typename Component>
	class remove_command : public command
	{
	private:
		entity & m_entity;
	public:
		remove_command(registry* registry, entity & entity)
			: command(registry)
			, m_entity(entity)
		{
		}

		void execute() override
		{
			m_registry->remove<Component>(m_entity);
		}
	};
}

#endif // !APOLLO_COMMAND_DESTROY_COMMAND
