#ifndef APOLLO_CORE_TYPE_TRAITS_H
#define APOLLO_CORE_TYPE_TRAITS_H

#include <tuple>

namespace apollo
{
	template <typename T>
	struct function_traits
		: public function_traits<decltype(&T::operator)>
	{
	};

	template <typename ClassType, typename ReturnType, typename... Args>
	struct function_traits<ReturnType(ClassType::*)(Args...) const>
	{
		using arity = std::integral_constant<std::size_t, sizeof...(Args)>;

		using result_type = ReturnType;

		template <size_t i>
		using arg = typename std::tuple_element<i, std::tuple<Args...>>::type;
	};
}

#endif // !APOLLO_CORE_TYPE_TRAITS_H
