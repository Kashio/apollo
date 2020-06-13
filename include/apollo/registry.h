#ifndef APOLLO_REGISTRY_H
#define APOLLO_REGISTRY_H

#include "core/type_traits.h"
#include "archetype.h"
#include "system.h"
#include "component.h"
#include "view.h"
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
		bool archetype_has_all_query_args(archetype* archetype, function_traits<ReturnType(ClassType::*)(Args...)const>)
		{
			return archetype->has_all<std::decay_t<Args>...>();
		}

		template<typename Fn, typename ClassType, typename ReturnType, typename... Args>
		void apply_to_archetype_components(archetype* archetype, Fn&& func, function_traits<ReturnType(ClassType::*)(Args...)const>)
		{
			std::tuple<std::vector<std::decay_t<Args>>&...> components{ *archetype->get_components<std::decay_t<Args>>()... };
			auto size = std::get<0>(components).size();
			for (std::size_t i = 0; i < size; ++i)
			{
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

		template <typename... TComponents>
		auto view()
		{
			return apollo::view<TComponents...>(this);
		}

		template <typename TSystem, typename... Args>
		const TSystem& create_system(Args&&... args)
		{
			static_assert(std::is_base_of<system, TSystem>::value, "type parameter of this class must derive from system");
			m_systems.push_back(std::make_unique<TSystem>(std::forward<Args>(args)...));
			return *dynamic_cast<TSystem*>(m_systems.back().get());
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
			new_archetype->set<TComponent>(entity, std::forward<Args>(args)...);
			context->move<TComponent>(*new_archetype, entity);
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
			}
		}

		template <typename... TComponents>
		void clear()
		{
			static_assert(((std::is_base_of<component<TComponents>, TComponents>::value) && ...), "type parameters TComponents must derive from component");
			for (std::size_t i = 0; i < m_entity_index.size(); ++i)
				((remove<TComponents>(i)), ...);
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

		template <typename TComponent>
		TComponent& get_at(const std::size_t archetype_index, const std::size_t entity_index)
		{
			archetype* context = m_archetypes[archetype_index].get();
			return context->get_component_at<TComponent>(entity_index);
		}

		template <typename TComponent>
		TComponent* try_get_at(const std::size_t archetype_index, const std::size_t entity_index)
		{
			archetype* context = m_archetypes[archetype_index].get();
			return context->try_get_component_at<TComponent>(entity_index);
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
		std::tuple<TComponents&...> get_at(const std::size_t archetype_index, const std::size_t entity_index)
		{
			archetype* context = m_archetypes[archetype_index].get();
			return context->get_components_at<TComponents...>(entity_index);
		}

		template <typename... TComponents>
		auto try_get_at(const std::size_t archetype_index, const std::size_t entity_index)
		{
			archetype* context = m_archetypes[archetype_index].get();
			return context->try_get_components_at<TComponents...>(entity_index);
		}

		inline const std::vector<std::unique_ptr<archetype>>& get_archetypes() const
		{
			return m_archetypes;
		}

		template <typename... TComponents>
		std::vector<std::size_t> get_archetype_indices() const
		{
			std::vector<std::size_t> archretype_indices;
			for (std::size_t i = 0; i < m_archetypes.size(); ++i)
			{
				auto& archetype = m_archetypes[i];
				if (archetype->has_all<std::decay_t<TComponents>...>() && archetype->get_entities().size() > 0)
				{
					archretype_indices.push_back(i);
				}
			}
			return archretype_indices;
		}
	};
}

#endif // !APOLLO_REGISTRY_H
