#ifndef APOLLO_SYSTEM_H
#define APOLLO_SYSTEM_H

#include "job/job.h"
#include "job/job_handle.h"
#include <iostream>
#include <sstream>

namespace apollo
{
	class registry;

	class system
	{
	private:
		registry& m_registry;
	protected:
		job m_dependency;
	protected:
		system(registry& registry)
			: m_registry(registry) {}

		template <typename Fn>
		job& for_each(Fn&& query)
		{
			std::stringstream s1;
			s1 << " waiting for " << m_dependency.m_id << "\n";
			m_dependency = m_registry.for_each(query, m_dependency);
			std::stringstream s2;
			s2 << m_dependency.m_id << s1.str();
			std::cout << s2.str();
			return m_dependency;
		}
	public:
		~system() = default;

		virtual void update() = 0;
	};
}

#endif // !APOLLO_SYSTEM_H
