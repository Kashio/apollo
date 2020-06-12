#ifndef APOLLO_VIEW_H
#define APOLLO_VIEW_H

#include "core/common.h"
#include <vector>
#include <tuple>
#include <algorithm>
#include <type_traits>

namespace apollo
{
	template <typename... TComponents>
	class view
	{
	private:
		template <bool Const = false>
		class entity_iterator
		{
		private:
			registry const * m_registry;
			std::vector<std::size_t> m_archetype_indices;
			std::size_t m_archetype_i;
			std::size_t m_entity_i;
		public:
			using value_type = entity;
			using pointer = std::conditional_t<Const, const entity* const, entity*>;
			using reference = std::conditional_t<Const, const entity& const, entity&>;
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::random_access_iterator_tag;

			explicit entity_iterator(const registry* registry, std::vector<std::size_t>& archetype_indices,
				const std::size_t archetype_i = 0, const std::size_t entity_i = 0)
				: m_registry(registry)
				, m_archetype_indices(std::move(archetype_indices))
				, m_archetype_i(archetype_i)
				, m_entity_i(entity_i)
			{}

			inline const std::vector<std::size_t>& get_archetype_indices() const
			{
				return m_archetype_indices;
			}

			inline const std::size_t& get_archetype_index() const
			{
				return m_archetype_i;
			}

			inline const std::size_t& get_entity_index() const
			{
				return m_entity_i;
			}

			template<bool _Const = Const>
			std::enable_if_t<!_Const, reference>
			operator*() const
			{
				return m_registry->get_archetypes()[m_archetype_indices[m_archetype_i]]->get_entities()[m_entity_i];
			}

			template<bool _Const = Const>
			std::enable_if_t<_Const, reference>
			operator*() const
			{
				return m_registry->get_archetypes()[m_archetype_indices[m_archetype_i]]->get_entities()[m_entity_i];
			}

			pointer operator->()
			{
				return &m_registry->get_archetypes()[m_archetype_indices[m_archetype_i]]->get_entities()[m_entity_i];
			}

			template<bool _Const = Const>
			std::enable_if_t<!_Const, reference>
			operator[](int m)
			{
				increment_index(m);
				return m_registry->get_archetypes()[m_archetype_indices[m_archetype_i]]->get_entities()[m_entity_i];
			}

			template<bool _Const = Const>
			std::enable_if_t<_Const, reference>
			operator[](int m) const
			{
				std::size_t new_archetype_i = m_archetype_i;
				std::size_t new_entity_i = m_entity_i;
				std::size_t num_of_entities = m_registry->get_archetypes()[m_archetype_indices[new_archetype_i]]->get_entities().size();
				while (new_entity_i + m >= num_of_entities)
				{
					m -= num_of_entities - new_entity_i - 1;
					new_entity_i = 0;
					++new_archetype_i;
					num_of_entities = m_registry->get_archetypes()[m_archetype_indices[new_archetype_i]]->get_entities().size();
				}
				new_entity_i += m;
				return m_registry->get_archetypes()[m_archetype_indices[new_archetype_i]]->get_entities()[new_entity_i];
			}

			entity_iterator& operator++()
			{
				increment_index(1);
				return *this;
			}

			entity_iterator& operator--()
			{
				decrement_index(1);
				return *this;
			}

			entity_iterator operator++(int)
			{
				entity_iterator it(*this);
				increment_index(-1);
				return it;
			}

			entity_iterator operator--(int)
			{
				entity_iterator it(*this);
				decrement_index(-1);
				return it;
			}

			entity_iterator& operator+=(int n)
			{
				increment_index(n);
				return *this;
			}

			entity_iterator& operator-=(int n)
			{
				decrement_index(n);
				return *this;
			}

			entity_iterator operator+(int n) const
			{
				entity_iterator it(*this);
				return it += n;
			}

			entity_iterator operator-(int n) const
			{
				entity_iterator it(*this);
				return it -= n;
			}

			difference_type operator-(entity_iterator const& it) const
			{
				return accumelative_index() - it.accumelative_index();
			}

			bool operator<(entity_iterator const& it) const
			{
				return accumelative_index() < it.accumelative_index();
			}

			bool operator<=(entity_iterator const& it) const
			{
				return accumelative_index() <= it.accumelative_index();
			}

			bool operator>(entity_iterator const& it) const
			{
				return accumelative_index() > it.accumelative_index();
			}

			bool operator>=(entity_iterator const& it) const
			{
				return accumelative_index() >= it.accumelative_index();
			}

			bool operator!=(const entity_iterator &it) const
			{
				return accumelative_index() != it.accumelative_index();
			}

			bool operator==(const entity_iterator &it) const
			{
				return accumelative_index() == it.accumelative_index();
			}
		private:
			void increment_index(int m)
			{
				std::size_t num_of_entities = m_registry->get_archetypes()[m_archetype_indices[m_archetype_i]]->get_entities().size();
				while (m_entity_i + m >= num_of_entities)
				{
					m -= num_of_entities - m_entity_i;
					m_entity_i = 0;
					if (m_archetype_i + 1 != m_archetype_indices.size())
					{
						++m_archetype_i;
					}
					else
					{
						m_entity_i = num_of_entities;
						return;
					}
					num_of_entities = m_registry->get_archetypes()[m_archetype_indices[m_archetype_i]]->get_entities().size();
				}
				m_entity_i += m;
			}

			void decrement_index(int m)
			{
				while (m_entity_i < m)
				{
					m -= m_entity_i;
					--m_archetype_i;
					m_entity_i = m_registry->get_archetypes()[m_archetype_indices[m_archetype_i]]->get_entities().size() - 1;
				}
				m_entity_i -= m;
			}

			std::size_t accumelative_index() const
			{
				std::size_t acc_i = 0;
				for (std::size_t i = 0; i < m_archetype_i; ++i)
				{
					acc_i += m_registry->get_archetypes()[m_archetype_indices[i]]->get_entities().size();
				}
				acc_i += m_entity_i;
				return acc_i;
			}
		};

		registry* m_registry;
	public:
		typedef entity_iterator<false> iterator;
		typedef entity_iterator<true> const_iterator;

		view(registry* registry)
			: m_registry(registry)
		{
		}

		template <typename TComponent>
		TComponent* get(const entity& entity)
		{
			entity_iterator<false> it = std::find(begin(), end(), entity);
			auto components = m_registry->get_archetypes()[it.get_archetype_indices()[it.get_archetype_index()]]->get<TComponent>();
			if (components)
				return &components->operator[](it.get_entity_index());
			return nullptr;
		}

		//template <typename... TComponents>
		//std::tuple<TComponents&...> get(const entity& entity)
		//{

		//}

		iterator begin()
		{
			return entity_iterator<false>(m_registry, m_registry->get_archetype_indices<TComponents...>());
		}

		const const_iterator begin() const
		{
			return entity_iterator<true>(m_registry, m_registry->get_archetype_indices<TComponents...>());
		}

		iterator end()
		{
			auto archetype_indices = m_registry->get_archetype_indices<TComponents...>();
			return entity_iterator<false>(m_registry, archetype_indices,
				archetype_indices.size() - 1, m_registry->get_archetypes()[archetype_indices[archetype_indices.size() - 1]]->get_entities().size());
		}

		const const_iterator end() const
		{
			auto archetype_indices = m_registry->get_archetype_indices<TComponents...>();
			return entity_iterator<true>(m_registry, archetype_indices,
				archetype_indices.size() - 1, m_registry->get_archetypes()[archetype_indices[archetype_indices.size() - 1]]->get_entities().size());
		}
	};
}

#endif // !APOLLO_VIEW_H
