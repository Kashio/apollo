#ifndef APOLLO_REGISTRY_H
#define APOLLO_REGISTRY_H

#include "core/type_traits.h"
#include "archetype.h"
#include "system.h"
#include "component.h"
#include <vector>
#include <unordered_map>
#include <memory>

namespace apollo
{
	class registry {
	private:
		std::vector<std::unique_ptr<archetype>> m_archetypes;
		std::vector<std::unique_ptr<system>> m_systems;
		std::unordered_map<entity, std::size_t> m_entity_index;
	private:
		template <typename ClassType, typename ReturnType, typename... Args>
		bool archetype_has_all_query_args(archetype* archetype, function_traits<ReturnType(ClassType::*)(Args...)>) {
			return archetype->has_all<std::decay_t<Args>...>();
		}

		template<typename Fn, typename ClassType, typename ReturnType, typename... Args>
		void apply_to_archetype_components(archetype* archetype, Fn&& func, function_traits<ReturnType(ClassType::*)(Args...)>) {
			std::tuple<std::vector<std::decay_t<Args>>&...> components{ *archetype->get<std::decay_t<Args>>()... };
			auto size = std::get<0>(components).size();
			for (std::size_t i = 0; i < size; ++i) {
				std::apply([&](auto&... vecs) {
					func(vecs[i]...);
				}, components);
			}
		}
	public:
		const entity& create()
		{
			static entity s_current_id = 0;
			return s_current_id++;
		}

		void update()
		{
			for (auto& system : m_systems)
			{
				system->update(this);
			}
		}

		template <typename Fn>
		void for_each(Fn&& fn)
		{
			typedef function_traits<decltype(fn)> traits;
			for (auto& archetype : m_archetypes)
			{
				if (archetype_has_all_query_args(archetype.get(), traits::self()))
				{
					apply_to_archetype_components(archetype.get(), fn, traits::self());
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

		template <typename TComponent, typename... Args>
		void emplace(entity entity, Args&&... args)
		{
			static_assert(std::is_base_of<component<TComponent>, TComponent>::value, "type parameter of this class must derive from component");
			auto it = m_entity_index.find(entity);
			if (it == m_entity_index.end())
			{
				// Entity doesn't belong to an Archetype
				// We create an Archetype and add a component to it

				// If AddComponent is called in different thread could cause problem
				// With id = m_Archetypes.size() - 1 and context switching happening before we finish this code block
				auto archetype = std::make_unique<archetype>(m_archetypes.size(), std::make_unique<component_storage_impl<TComponent>>());
				archetype->add(entity);
				archetype->set<TComponent>(entity, std::forward<Args>(args)...);
				m_archetypes.push_back(std::move(archetype));
				m_entity_index[entity] = m_archetypes.size() - 1;
			}
			else
			{
				// Entity does belong to an Archetype
				// We check if new Archetype already exists or we should create it
				archetype* archetype = m_archetypes[it->second].get();

				archetype* new_archetype = archetype->get_edge(TComponent::ID);

				if (!new_archetype)
				{
					new_archetype = archetype->with_added_component<TComponent>(m_archetypes.size());
					m_archetypes.emplace_back(new_archetype);
					m_entity_index[entity] = m_archetypes.size() - 1;
				}
				else
				{
					m_entity_index[entity] = new_archetype->get_id();
				}

				new_archetype->add(entity);
				new_archetype->set<TComponent>(entity, std::forward<Args>(args)...);
				archetype->move(*new_archetype, entity);
			}
		}

		template <typename TComponent>
		void remove(Entity entity)
		{
			static_assert(std::is_base_of<component<TComponent>, TComponent>::value, "type parameter of this class must derive from component");
			auto it = m_EntityIndex.find(entity);
			if (it != m_EntityIndex.end())
			{
				// Entity does belong to an Archetype
				// We remove the entity from the Archetype
				archetype* archetype = m_archetypes[it->second].get();

				archetype* new_archetype = archetype->get_ddge(TComponent::ID);

				if (!new_archetype)
				{
					new_archetype = archetype->with_removed_component<TComponent>(m_archetypes.size());
					m_archetypes.emplace_back(new_archetype);
					m_entity_index[entity] = m_archetypes.size() - 1;
				}
				else
				{
					m_entity_index[entity] = new_archetype->get_id();
				}

				new_archetype->add(entity);
				archetype->move<TComponent>(*new_archetype, entity);
			}
		}
	};
}
	
#endif // !APOLLO_REGISTRY_H
