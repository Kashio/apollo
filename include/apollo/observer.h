#ifndef APOLLO_OBSERVER_H
#define APOLLO_OBSERVER_H

#include "core/common.h"
#include <unordered_map>
#include <functional>

namespace apollo
{
	using Callback = std::function<void(registry&, entity const &)>;

	class observer
	{
	private:
		std::unordered_map<id_type, Callback> m_callbacks;
	public:
		id_type connect(Callback&& callback)
		{
			static id_type id = 0;
			m_callbacks[id] = std::move(callback);
			return id++;
		}

		void disconnect(id_type id)
		{
			auto it = m_callbacks.find(id);
			if (it != m_callbacks.end())
				m_callbacks.erase(id);
		}

		void notify(registry& r, entity const & e)
		{
			for (auto& callback : m_callbacks)
				callback.second.operator()(r, e);
		}
	};
}

#endif // !APOLLO_OBSERVER_H
