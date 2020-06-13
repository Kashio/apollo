#ifndef APOLLO_COMMAND_COMMAND_BUFFER_H
#define APOLLO_COMMAND_COMMAND_BUFFER_H

#include <vector>
#include <memory>
#include "command.h"

namespace apollo
{
	class command_buffer
	{
	private:
		std::vector<std::unique_ptr<command>> m_commands;
		registry* m_registry;
	public:
		command_buffer(registry* registry)
			: m_registry(registry)
		{
		}

		template <typename TCommand, typename... Args>
		void add_command(Args&&... args)
		{
			static_assert(std::is_base_of<command, TCommand>::value, "type parameter TCommand must derive from apollo::command");
			m_commands.push_back(std::make_unique<TCommand>(m_registry, std::forward<Args>(args)...));
		}

		void execute()
		{
			for (auto& command : m_commands)
				command->execute();
		}
	};
}

#endif // !APOLLO_COMMAND_COMMAND_BUFFER_H
