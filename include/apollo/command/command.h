#ifndef APOLLO_COMMAND_COMMAND_H
#define APOLLO_COMMAND_COMMAND_H

namespace apollo
{
	class registry;

	class command
	{
	protected:
		registry* m_registry;
	protected:
		command(registry* registry)
			: m_registry(registry)
		{
		}
	public:
		virtual ~command()
		{
		}

		virtual void execute() = 0;
	};
}

#endif // !APOLLO_COMMAND_COMMAND_H
