#ifndef APOLLO_REGISTRY_H
#define APOLLO_REGISTRY_H

#include "core/type_traits.h"
#include "archetype.h"
#include "system.h"
#include "component.h"
#include "observer.h"
#include "command/command_buffer.h"
#include <vector>
#include <unordered_map>
#include <tuple>
#include <memory>

namespace apollo
{
	class registry {
	private:
		std::vector<std::unique_ptr<archetype>> m_archetypes;
		std::vector<std::unique_ptr<system>> m_systems;
		std::vector<std::size_t> m_entity_index;
		std::vector<entity> destroyed_entities;
		std::unordered_map<id_type, observer> m_on_construct_observers;
		std::unordered_map<id_type, observer> m_on_destroy_observers;
		std::unordered_map<id_type, observer> m_on_update_observers;
	private:
		template <typename ClassType, typename ReturnType, typename Entity, typename... Args>
		bool archetype_has_all_query_args_with_entity(archetype* archetype, function_traits<ReturnType(ClassType::*)(Entity, Args...)const>)
		{
			return archetype->has_all<std::decay_t<Args>...>();
		}

		template <typename ClassType, typename ReturnType, typename... Args>
		bool archetype_has_all_query_args_without_entity(archetype* archetype, function_traits<ReturnType(ClassType::*)(Args...)const>)
		{
			return archetype->has_all<std::decay_t<Args>...>();
		}

		template<typename Fn, typename ClassType, typename ReturnType, typename Entity, typename... Args>
		void apply_to_archetype_components(archetype* archetype, Fn&& func, function_traits<ReturnType(ClassType::*)(Entity, Args...)const>)
		{
			std::tuple<std::vector<std::decay_t<Args>>&...> components{ *archetype->get_components<std::decay_t<Args>>()... };
			auto size = std::get<0>(components).size();
			for (std::size_t i = 0; i < size; ++i)
			{
				std::apply([&](auto&... vecs) {
					func(archetype->m_entities[i], vecs[i]...);
				}, components);
			}
		}

		template<typename Fn, typename ClassType, typename ReturnType, typename... Args>
		void apply_to_archetype_entity_components(archetype* archetype, Fn&& func, function_traits<ReturnType(ClassType::*)(Args...)const>, const entity& entity)
		{
			auto components = archetype->get_components<std::decay_t<Args>...>(entity);
			std::apply([&](auto&... component) {
				func(component...);
			}, components);
		}

		template<typename Component>
		void apply_to_on_update_observers(const entity& entity)
		{
			auto it = m_on_update_observers.find(Component::id);
			if (it != m_on_update_observers.end())
				it->second.notify(*this, entity);
		}

		template<typename ClassType, typename ReturnType, typename... Args>
		void apply_to_on_update_observers(const entity& entity, function_traits<ReturnType(ClassType::*)(Args...)const>)
		{
			((apply_to_on_update_observers<std::decay_t<Args>>(entity)), ...);
		}

		archetype* find_archetype_with_same_signature(archetype& archetpye)
		{
			for (const auto& a : m_archetypes)
			{
				if (*a.get() == archetpye)
					return a.get();
			}
			return nullptr;
		}
	public:
		registry()
		{
			auto empty_archetype = new archetype(m_archetypes.size());
			m_archetypes.emplace_back(empty_archetype);
		}

		template <typename Component>
		observer& on_construct()
		{
			auto it = m_on_construct_observers.find(Component::id);
			if (it == m_on_construct_observers.end())
				m_on_construct_observers[Component::id] = observer();
			return m_on_construct_observers[Component::id];
		}

		template <typename Component>
		observer& on_destroy()
		{
			auto it = m_on_destroy_observers.find(Component::id);
			if (it == m_on_destroy_observers.end())
				m_on_destroy_observers[Component::id] = observer();
			return m_on_destroy_observers[Component::id];
		}

		template <typename Component>
		observer& on_update()
		{
			auto it = m_on_destroy_observers.find(Component::id);
			if (it == m_on_destroy_observers.end())
				m_on_destroy_observers[Component::id] = observer();
			return m_on_destroy_observers[Component::id];
		}

		const entity& create()
		{
			static entity s_current_id = 0;
			entity current;
			if (destroyed_entities.size())
			{
				current = destroyed_entities.back();
				destroyed_entities.pop_back();
				m_entity_index[current] = 0;
			}
			else
			{
				current = s_current_id++;
				m_entity_index.push_back(0);
			}
			return current;
		}

		void destroy(entity& entity)
		{
			archetype* context = m_archetypes[m_entity_index[entity]].get();

			context->remove(entity);
			m_entity_index[entity] = invalid_index;
			destroyed_entities.push_back(entity);

			for (std::size_t i = 0; i < context->m_signature.size(); ++i)
			{
				if (context->m_signature[i] != invalid_index)
				{
					auto it = m_on_destroy_observers.find(i);
					if (it != m_on_destroy_observers.end())
						it->second.notify(*this, entity);
				}
			}
		}

		bool valid(const entity& entity)
		{
			return m_entity_index[entity] == invalid_index;
		}

		void update()
		{
			for (auto& system : m_systems)
			{
				system->update(*this);
			}
		}

		template <typename Fn>
		void for_each(Fn&& fn)
		{
			typedef function_traits<decltype(fn)> traits;
			auto t = traits::self();
			static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<traits::arg<0>>>, entity>::value, "first type parameter of query must be of type apollo::entity");
			for (auto& archetype : m_archetypes)
			{
				if (archetype_has_all_query_args_with_entity(archetype.get(), t))
				{
					apply_to_archetype_components(archetype.get(), fn, t);
				}
			}
		}

		template <typename TSystem, typename... Args>
		const TSystem& create_system(Args&&... args)
		{
			static_assert(std::is_base_of<system, TSystem>::value, "type parameter of this class must derive from system");
			m_systems.push_back(std::make_unique<TSystem>(std::forward<Args>(args)...));
			return *dynamic_cast<TSystem*>(m_systems.back().get());
		}

		command_buffer create_command_buffer()
		{
			return command_buffer(this);
		}

		template <typename TComponent, typename... Args>
		TComponent& emplace(entity entity, Args&&... args)
		{
			static_assert(std::is_base_of<component<TComponent>, TComponent>::value, "type parameter of this class must derive from component");
			archetype* context = m_archetypes[m_entity_index[entity]].get();
			archetype* new_archetype = context->get_edge(TComponent::id);

			if (!new_archetype)
			{
				archetype temp{ 0 };
				temp.m_signature = context->m_signature;
				temp.add_to_signature(0, TComponent::id);

				archetype* existing_archetype = find_archetype_with_same_signature(temp);
				if (!existing_archetype)
				{
					new_archetype = context->with_added_component<TComponent>(m_archetypes.size());
					m_archetypes.emplace_back(new_archetype);
				}
				else
				{
					new_archetype = existing_archetype;
					context->m_edges[TComponent::id] = existing_archetype;
					existing_archetype->m_edges[TComponent::id] = context;
				}
			}
			m_entity_index[entity] = new_archetype->get_id();
			new_archetype->add(entity);
			new_archetype->set_at<TComponent>(new_archetype->m_entities.size() - 1, std::forward<Args>(args)...);
			context->move<TComponent>(*new_archetype, entity);

			auto it = m_on_construct_observers.find(TComponent::id);
			if (it != m_on_construct_observers.end())
				it->second.notify(*this, entity);

			return new_archetype->get_component_at<TComponent>(new_archetype->m_entities.size() - 1);
		}

		template <typename TComponent>
		void remove(entity entity)
		{
			static_assert(std::is_base_of<component<TComponent>, TComponent>::value, "type parameter of this class must derive from component");
			if (m_entity_index[entity])
			{
				archetype* context = m_archetypes[m_entity_index[entity]].get();
				if (!context->has_all<TComponent>())
					return;
				archetype* new_archetype = context->get_edge(TComponent::id);

				if (!new_archetype)
				{
					archetype temp{ 0 };
					temp.m_signature = context->m_signature;
					temp.remove_from_signature(TComponent::id);

					archetype* existing_archetype = find_archetype_with_same_signature(temp);
					if (!existing_archetype)
					{
						new_archetype = context->with_removed_component<TComponent>(m_archetypes.size());
						m_archetypes.emplace_back(new_archetype);
					}
					else
					{
						new_archetype = existing_archetype;
						context->m_edges[TComponent::id] = existing_archetype;
						existing_archetype->m_edges[TComponent::id] = context;
					}
				}
				m_entity_index[entity] = new_archetype->get_id();
				new_archetype->add(entity);
				context->move<TComponent>(*new_archetype, entity);

				auto it = m_on_destroy_observers.find(TComponent::id);
				if (it != m_on_destroy_observers.end())
					it->second.notify(*this, entity);
			}
		}

		template <typename... TComponents>
		void clear()
		{
			static_assert(((std::is_base_of<component<TComponents>, TComponents>::value) && ...), "type parameters TComponents must derive from component");
			for (std::size_t i = 0; i < m_entity_index.size(); ++i)
				((remove<TComponents>(i)), ...);
		}

		void clear()
		{
			for (std::size_t i = 0; i < m_entity_index.size(); ++i)
			{
				if (m_entity_index[i] != invalid_index && m_entity_index[i] != 0)
					destroy(i);
			}
		}

		template <typename Fn>
		void patch(const entity& entity, Fn&& fn)
		{
			if (m_entity_index[entity])
			{
				archetype* context = m_archetypes[m_entity_index[entity]].get();
				typedef function_traits<decltype(fn)> traits;
				if (archetype_has_all_query_args_without_entity(context, traits::self()))
				{
					apply_to_archetype_entity_components(context, fn, traits::self(), entity);
					apply_to_on_update_observers(entity, traits::self());
				}
			}
		}

		template <typename TComponent, typename... Args>
		void replace(const entity& entity, Args&&... args)
		{
			if (m_entity_index[entity])
			{
				archetype* context = m_archetypes[m_entity_index[entity]].get();
				context->set<TComponent>(entity, std::forward<Args>(args)...);

				auto it = m_on_update_observers.find(TComponent::id);
				if (it != m_on_update_observers.end())
					it->second.notify(*this, entity);
			}
		}

		template <typename... TComponents>
		bool has(const entity& entity)
		{
			static_assert(((std::is_base_of<component<TComponents>, TComponents>::value) && ...), "type parameters TComponents must derive from component");
			archetype* context = m_archetypes[m_entity_index[entity]].get();
			return context->has_all<TComponents...>();
		}

		template <typename... TComponents>
		bool any(const entity& entity)
		{
			static_assert(((std::is_base_of<component<TComponents>, TComponents>::value) && ...), "type parameters TComponents must derive from component");
			archetype* context = m_archetypes[m_entity_index[entity]].get();
			return ((context->has_all<TComponents>()) || ...);
		}

		template <typename TComponent>
		TComponent& get(const entity& entity)
		{
			archetype* context = m_archetypes[m_entity_index[entity]].get();
			return context->get_component<TComponent>(entity);
		}

		template <typename TComponent>
		TComponent* try_get(const entity& entity)
		{
			archetype* context = m_archetypes[m_entity_index[entity]].get();
			return context->try_get_component<TComponent>(entity);
		}

		template <typename... TComponents>
		std::tuple<TComponents&...> get(const entity& entity)
		{
			archetype* context = m_archetypes[m_entity_index[entity]].get();
			return context->get_components<TComponents...>(entity);
		}

		template <typename... TComponents>
		auto try_get(const entity& entity)
		{
			archetype* context = m_archetypes[m_entity_index[entity]].get();
			return context->try_get_components<TComponents...>(entity);
		}

		template <typename... TComponents>
		std::vector<entity> get_entities() const
		{
			std::vector<std::size_t> entities;
			for (std::size_t i = 1; i < m_archetypes.size(); ++i)
			{
				auto& archetype = m_archetypes[i];
				if (archetype->has_all<TComponents...>())
				{
					entities.insert(archetype->m_entities.begin(), archetype->m_entities.end());
				}
			}
			return entities;
		}

		std::vector<entity> get_entities() const
		{
			std::vector<std::size_t> entities;
			for (std::size_t i = 0; i < m_entity_index.size(); ++i)
			{
				if (m_entity_index[i] != invalid_index)
					entities.push_back(i);
			}
			return entities;
		}

		std::vector<entity> get_orphan_entities() const
		{
			std::vector<std::size_t> entities;
			for (std::size_t i = 0; i < m_entity_index.size(); ++i)
			{
				if (m_entity_index[i] == 0)
					entities.push_back(i);
			}
			return entities;
		}


	};
}

#endif // !APOLLO_REGISTRY_H
