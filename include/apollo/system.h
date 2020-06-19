#ifndef APOLLO_SYSTEM_H
#define APOLLO_SYSTEM_H

namespace apollo
{
	class registry;

	class system
	{
	protected:
		system() = default;
	public:
		~system() = default;
		virtual void update(registry& registry) = 0;
	};
}

#endif // !APOLLO_SYSTEM_H
