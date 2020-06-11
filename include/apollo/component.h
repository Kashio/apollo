#ifndef APOLLO_COMPONENT_H
#define APOLLO_COMPONENT_H

#include "core/common.h"
#include <atomic>

namespace apollo
{
	static std::atomic<id_type> current_id = 0;

	template <typename T>
	class component
	{
	public:
		inline static const id_type id = current_id++;
	};
};

#endif // !APOLLO_COMPONENT_H