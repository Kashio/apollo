#ifndef APOLLO_REGISTRY_H
#define APOLLO_REGISTRY_H

#include "core/type_traits.h"
#include "archetype.h"
#include "system.h"
#include "component.h"
#include <vector>
#include <tuple>
#include <memory>

namespace apollo
{
	class registry {
	private:
		std::vector<std::unique_ptr<archetype>> m_archetypes;
		std::vector<std::unique_ptr<system>> m_systems;
		std::vector<std::size_t> m_entity_index;
	private:
		template <typename ClassType, typename ReturnType, typename... Args>
		bool archetype_has_all_query_args(archetype* archetype, function_traits<ReturnType(ClassType::*)(Args...)const>) {
			return archetype->has_all<std::decay_t<Args>...>();
		}

		template<typename Fn, typename ClassType, typename ReturnType, typename... Args>
		void apply_to_archetype_components(archetype* archetype, Fn&& func, function_traits<ReturnType(ClassType::*)(Args...)const>) {
			std::tuple<std::vector<std::decay_t<Args>>&...> components{ *archetype->get<std::decay_t<Args>>()... };
			auto size = std::get<0>(components).size();
			for (std::size_t i = 0; i < size; ++i) {
				std::apply([&](auto&... vecs) {
					func(vecs[i]...);
				}, components);
			}
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

		const entity& create()
		{
			static entity s_current_id = 0;
			entity current = s_current_id++;
			m_entity_index.push_back(0);
			return current;
		}

		void destroy(const entity& entity)
		{
			m_archetypes[m_entity_index[entity]]->remove(entity);
			m_entity_index[entity] = 0;
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
			new_archetype->set<TComponent>(entity, std::forward<Args>(args)...);
			context->move<TComponent>(*new_archetype, entity);
		}

		template <typename TComponent>
		void remove(entity entity)
		{
			static_assert(std::is_base_of<component<TComponent>, TComponent>::value, "type parameter of this class must derive from component");
			if (m_entity_index[entity])
			{
				archetype* context = m_archetypes[m_entity_index[entity]].get();
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
			}
		}
	};
}
	
#endif // !APOLLO_REGISTRY_H
