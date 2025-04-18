#pragma once
#include <svh/visit_struct/visit_struct.hpp>
#include <type_traits>

namespace svh {

	/* Generic checks */
	template<typename Test, template<typename...> class Ref>
	struct is_specialization : std::false_type {};

	template<template<typename...> class Ref, typename... Args>
	struct is_specialization<Ref<Args...>, Ref> : std::true_type {};

	template<typename T> /* Has a visitable struct implementation */
	using is_visitable = std::integral_constant<bool, visit_struct::traits::is_visitable<T>::value>;

	template<typename T, typename R>
	using enable_if_visitable = std::enable_if_t<is_visitable<T>::value, R>;

	template<typename T> /* Is considerd to be arithmetic */
	using is_number = std::integral_constant<bool, std::is_arithmetic<T>::value>;

	template<typename T, typename R>
	using enable_if_number = std::enable_if_t<is_number<T>::value, R>;

	template<typename T> /* Is considerd to be an enum */
	using is_enum = std::integral_constant<bool, std::is_enum<T>::value>;

	template<typename T, typename R>
	using enable_if_enum = std::enable_if_t<is_enum<T>::value, R>;

#pragma region external
	// Source: https://en.cppreference.com/w/cpp/experimental/is_detected

	namespace detail {

		struct nonesuch {
			nonesuch() = delete;
			~nonesuch() = delete;
			nonesuch(nonesuch const&) = delete;
			nonesuch(nonesuch&&) = delete;
			void operator=(nonesuch const&) = delete;
			void operator=(nonesuch&&) = delete;
		};

		template<class Default, class AlwaysVoid, template<class...> class Op, class... Args>
		struct detector {
			using value_t = std::false_type;
			using type = Default;
		};

		template<class Default, template<class...> class Op, class... Args>
		struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> {
			using value_t = std::true_type;
			using type = Op<Args...>;
		};
	} // namespace detail

	template<template<class...> class Op, class... Args>
	using is_detected = typename detail::detector<detail::nonesuch, void, Op, Args...>::value_t;

	template<template<class...> class Op, class... Args>
	using detected_t = typename detail::detector<detail::nonesuch, void, Op, Args...>::type;

	template<class Default, template<class...> class Op, class... Args>
	using detected_or = detail::detector<Default, void, Op, Args...>;
#pragma endregion

	template <typename T>
	using serialize_fn = decltype(SerializeImpl(std::declval<const T&>()));

	template<typename T> /* Has a user defined serialize function */
	using has_serialize = std::integral_constant<bool, is_detected<serialize_fn, T>::value>;

	template <typename T, typename R>
	using enable_if_has_serialize = std::enable_if_t<has_serialize<T>::value, R>;
}