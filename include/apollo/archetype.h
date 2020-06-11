#ifndef APOLLO_ARCHETYPE_H
#define APOLLO_ARCHETYPE_H

#include <algorithm>
#include "component_storage.h"

namespace apollo
{
	using storage_vec = std::vector<std::unique_ptr<component_storage>>;

	const int invalid_storage_index = -1;

	class archetype
	{
	private:
		id_type m_id;
		std::size_t m_num_components = 0;
		std::vector<int> m_signature;
		storage_vec m_storages;
		std::vector<entity> m_entities;
		std::vector<archetype*> m_edges;
	private:
		explicit archetype(const id_type id, storage_vec& storages)
			: m_id(id), m_storages(std::move(storages))
		{
			for (std::size_t i = 0; i < m_storages.size(); ++i) {
				add_to_signature(i, m_storages[i]->get_id());
			}
		}

		void add_to_signature(const std::size_t index, id_type component_id)
		{
			if (m_signature.size() <= component_id) {
				m_signature.resize(component_id + 1, invalid_storage_index);
			}
			m_signature[component_id] = index;
			++m_num_components;
		}

		template<typename Component>
		inline component_storage_impl<Component>* get_storage()
		{
			int index = m_signature[Component::id];
			if (index == invalid_storage_index)
			{
				return nullptr;
			}
			auto& s = m_storages[index];
			return static_cast<component_storage_impl<Component>*>(s.get());
		}

		inline component_storage* get_storage(const id_type component_id)
		{
			int index = m_signature[component_id];
			if (index == invalid_storage_index)
			{
				return nullptr;
			}
			auto& s = m_storages[index];
			return s.get();
		}
	public:
		archetype(const id_type id)
			: m_id(id)
		{
		}

		archetype(const id_type id, std::unique_ptr<component_storage>&& storage)
			: m_id(id)
		{
			m_storages.push_back(std::move(storage));
			add_to_signature(0, m_storages[0]->get_id());
		}

		inline const id_type get_id() const
		{
			return m_id;
		}

		inline archetype* get_edge(const std::size_t component_id)
		{
			if (component_id < m_edges.size())
				return m_edges[component_id];
			return nullptr;
		}

		inline const std::size_t get_num_components() const
		{
			return m_num_components;
		}

		template<typename Component>
		std::vector<Component>* get()
		{
			auto storage = get_storage<Component>();
			if (storage)
			{
				return &(storage->m_components);
			}
			return nullptr;
		}

		void add(entity entity)
		{
			std::for_each(m_storages.begin(), m_storages.end(), [](auto&& s) {
				s->add();
			});
			m_entities.resize(m_entities.size() + 1, entity);
		}

		inline std::size_t search(entity entity)
		{
			return std::distance(m_entities.begin(), std::find(m_entities.begin(), m_entities.end(), entity));
		}

		template<typename Component, typename... Args>
		void set(entity entity, Args&&... args)
		{
			auto storage = get_storage<Component>();
			if (storage)
			{
				std::size_t index = search(entity);
				if (index >= m_entities.size())
					return;
				storage->m_components[index] = Component(std::forward<Args>(args)...);
			}
		}

		void remove(entity entity)
		{
			// TODO: doing sparse like set removal which is fast for adding/removing entities from Archetype
			// Should implement a config variable to decide between fast addition/removal of entities or fast entities lookup
			// by sorting the entities vector
			std::size_t index = search(entity);
			if (index >= m_entities.size())
				return;
			std::for_each(m_storages.begin(), m_storages.end(), [&index](auto&& s) {
				s->remove(index);
			});
			m_entities[index] = m_entities.back();
			m_entities.resize(m_entities.size() - 1);
		}

		template <typename... TComponent>
		void copy(archetype& destination, entity entity)
		{
			const std::size_t index = search(entity);
			if (index >= m_entities.size())
				return;
			std::size_t i = 0;
			std::for_each(m_storages.begin(), m_storages.end(), [&destination, &index, &i](auto&& s) {
				if (((s->get_id() != TComponent::id) && ...))
				{
					s->copy(*destination.get_storage(s->get_id()), index);
				}
				++i;
			});
		}

		template <typename... TComponent>
		void move(archetype& destination, entity entity)
		{
			const std::size_t index = search(entity);
			if (index >= m_entities.size())
				return;
			std::size_t i = 0;
			std::for_each(m_storages.begin(), m_storages.end(), [&destination, &index, &i](auto&& s) {
				if (((s->get_id() != TComponent::id) && ...))
				{
					s->move(*destination.get_storage(s->get_id()), index);
				}
				s->remove(index);
				++i;
			});
			m_entities[index] = m_entities.back();
			m_entities.resize(m_entities.size() - 1);
		}

		template<typename... Component>
		bool has_all()
		{
			return ((Component::id < m_signature.size() && m_signature[Component::id] != invalid_storage_index) && ...);
		}

		template<typename Component>
		archetype* with_added_component(id_type id)
		{
			if (!get_edge(Component::id))
			{
				storage_vec storage;
				storage.reserve(m_storages.size() + 1);

				std::transform(m_storages.begin(), m_storages.end(), back_inserter(storage), [](auto&& s) {
					return s->create();
				});

				storage.push_back(std::make_unique<component_storage_impl<Component>>());

				m_edges.resize(Component::id + 1, nullptr);
				m_edges[Component::id] = new archetype(id, storage);
				m_edges[Component::id]->m_edges.resize(Component::id + 1, nullptr);
				m_edges[Component::id]->m_edges[Component::id] = this;
			}
			return m_edges[Component::id];
		}

		template<typename Component>
		archetype* with_removed_component(id_type id)
		{
			if (!get_edge(Component::id))
			{
				storage_vec storage;
				storage.reserve(m_storages.size() - 1);

				std::for_each(m_storages.begin(), m_storages.end(), [&storage](auto&& s)
				{
					if (s->get_id() != Component::id)
					{
						storage.push_back(s->create());
					}
				});

				//m_edges.resize(Component::id + 1, nullptr);
				m_edges[Component::id] = new archetype(id, storage);
				m_edges[Component::id]->m_edges.resize(Component::id + 1, nullptr);
				m_edges[Component::id]->m_edges[Component::id] = this;
			}
			return m_edges[Component::id];
		}

		friend class registry;
		friend bool operator==(const archetype& lhs, const archetype& rhs);
		friend bool operator!=(const archetype& lhs, const archetype& rhs);
	};

	bool operator==(const archetype& lhs, const archetype& rhs)
	{
		if (lhs.m_signature.size() != rhs.m_signature.size())
			return false;
		for (std::size_t i = 0; i < lhs.m_signature.size(); ++i)
		{
			if ((lhs.m_signature[i] == invalid_storage_index && rhs.m_signature[i] != invalid_storage_index) ||
				(lhs.m_signature[i] != invalid_storage_index && rhs.m_signature[i] == invalid_storage_index))
				return false;
		}
		return true;
	}

	bool operator!=(const archetype& lhs, const archetype& rhs)
	{
		return !(lhs == rhs);
	}
}

#endif // !APOLLO_ARCHETYPE_H
