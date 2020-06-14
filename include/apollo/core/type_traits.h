#ifndef APOLLO_CORE_TYPE_TRAITS_H
#define APOLLO_CORE_TYPE_TRAITS_H

#include <tuple>
#include <type_traits>

namespace apollo
{
	template <typename T>
	struct function_traits
		: public function_traits<decltype(&std::remove_reference_t<T>::operator())>
	{
		using self = function_traits<decltype(&std::remove_reference_t<T>::operator())>;
	};

	template <typename ClassType, typename ReturnType, typename... Args>
	struct function_traits<ReturnType(ClassType::*)(Args...) const>
	{
		using arity = std::integral_constant<std::size_t, sizeof...(Args)>;

		using result_type = ReturnType;

		template <std::size_t i>
		using arg = typename std::tuple_element_t<i, std::tuple<Args...>>;
	};
}

#endif // !APOLLO_CORE_TYPE_TRAITS_H
