#ifndef APOLLO_COMMAND_DESTROY_COMMAND
#define APOLLO_COMMAND_DESTROY_COMMAND

#include "command.h"
#include "../core/common.h"

namespace apollo
{
	template <typename... Components>
	class clear_command : public command
	{
	public:
		remove_command(registry* registry)
			: command(registry)
		{
		}

		void execute() override
		{
			m_registry->clear<Components...>(m_entity);
		}
	};
}

#endif // !APOLLO_COMMAND_DESTROY_COMMAND
