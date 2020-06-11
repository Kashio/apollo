#ifndef APOLLO_COMPONENT_STORAGE_H
#define APOLLO_COMPONENT_STORAGE_H

#include "core/common.h"
#include <memory>
#include <vector>

namespace apollo
{
	class component_storage
	{
	protected:
		component_storage() = default;
	public:
		virtual id_type get_id() const = 0;
		virtual std::unique_ptr<component_storage> create() const = 0;
		virtual void add() = 0;
		virtual void remove(const std::size_t index) = 0;
		virtual void copy(component_storage& destination, const std::size_t index) = 0;
		virtual void move(component_storage& destination, const std::size_t index) = 0;
	};

	template <typename Component>
	class component_storage_impl : public component_storage
	{
	public:
		using value_type = Component;
		std::vector<Component> m_components;
	public:
		inline id_type get_id() const override
		{
			return Component::id;
		}

		std::unique_ptr<component_storage> create() const override
		{
			return std::make_unique<component_storage_impl>();
		}

		void add() override
		{
			m_components.resize(m_components.size() + 1, Component());
		}

		void remove(const std::size_t index) override
		{
			std::swap(m_components[index], m_components.back());
			m_components.resize(m_components.size() - 1);
		}

		void copy(component_storage& destination, const std::size_t index) override
		{
			static_cast<component_storage_impl&>(destination).m_components.back() = m_components[index];
		}

		void move(component_storage& destination, const std::size_t index) override
		{
			static_cast<component_storage_impl&>(destination).m_components.back() = std::move(m_components[index]);
		}
	};
}

#endif // !APOLLO_COMPONENT_STORAGE_H