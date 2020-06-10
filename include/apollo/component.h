#ifndef APOLLO_COMPONENT_H
#define APOLLO_COMPONENT_H

#include "core/common.h"
#include <atomic>

namespace apollo
{
	static std::atomic<id_type> s_CurrentId;

	template <typename T>
	class component
	{
	public:
		inline static const id_type ID = s_CurrentId++;
	};
};

#endif // !APOLLO_COMPONENT_H