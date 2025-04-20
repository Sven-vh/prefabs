#pragma once
#include <svh/visit_struct/visit_struct.hpp>
#include <type_traits>

namespace svh {
	/* Still uses nlohmann json but so we can easily swap it for non ordered if wanted */
	using json = nlohmann::ordered_json;

	/* Generic checks */
	template<typename Test, template<typename...> class Ref>
	struct is_specialization : std::false_type {};

	template<template<typename...> class Ref, typename... Args>
	struct is_specialization<Ref<Args...>, Ref> : std::true_type {};

	template<typename T> /* Has a visitable struct implementation */
	constexpr bool is_visitable_v = visit_struct::traits::is_visitable<T>::value;

	template<typename T, typename R = void>
	using enable_if_visitable = std::enable_if_t<is_visitable_v<T>, R>;

	template<typename T> /* Is considerd to be arithmetic */
	constexpr bool is_number_v = std::is_arithmetic<T>::value;

	template<typename T, typename R = void>
	using enable_if_number = std::enable_if_t<is_number_v<T>, R>;

	template<typename T> /* Is considerd to be an enum */
	constexpr bool is_enum_v = std::is_enum<T>::value;

	template<typename T, typename R = void>
	using enable_if_enum = std::enable_if_t<is_enum_v<T>, R>;

	/* Has std::begin/std::end */
	template<class, class = void>
	struct has_begin_end : std::false_type {};

	template<class T>
	struct has_begin_end<
		T, std::void_t< decltype(std::begin(std::declval<T&>())), decltype(std::end(std::declval<T&>()))>> : std::true_type {
	};

	template<class T>
	constexpr bool has_begin_end_v = has_begin_end<T>::value;

	template<class T, class R = void>
	using enable_if_has_begin_end = std::enable_if_t<has_begin_end_v<T>, R>;

	/* can emplace back */
	template<class, class = void>
	struct has_emplace_back : std::false_type {};

	template<class T>
	struct has_emplace_back<
		T, std::void_t< decltype(std::declval<T>().emplace_back(std::declval<typename T::value_type>()))>> : std::true_type {
	};

	template<class T>
	constexpr bool has_emplace_back_v = has_emplace_back<T>::value;

	template<class T, class R = void>
	using enable_if_has_emplace_back = std::enable_if_t<has_emplace_back_v<T>, R>;

	template<class, class = void>
	struct has_insert : std::false_type {};

	template<class T>
	struct has_insert<
		T,
		std::void_t<
		decltype(
			std::declval<T>().insert(
				std::declval<typename T::iterator>(),
				std::declval<typename T::value_type>()
			)
			)
		>
	> : std::true_type {
	};

	template<class T>
	constexpr bool has_insert_v = has_insert<T>::value;

	template<class T, class R = void>
	using enable_if_has_insert = std::enable_if_t<has_insert_v<T>, R>;

	/* has emplace back */
	template<class, class = void>
	struct has_emplace : std::false_type {};

	template<class T>
	struct has_emplace<
		T, std::void_t< decltype(std::declval<T>().emplace(std::declval<typename T::value_type>()))>> : std::true_type {
	};

	template<class T>
	constexpr bool has_emplace_v = has_emplace<T>::value;

	template<class T, class R = void>
	using enable_if_has_emplace = std::enable_if_t<has_emplace_v<T>, R>;

	/* has emplace after */
	template<class, class = void>
	struct has_emplace_after : std::false_type {};

	template<class T>
	struct has_emplace_after<
		T,
		std::void_t<decltype(
			std::declval<T>().emplace_after(
				std::declval<typename T::const_iterator>(),
				std::declval<typename T::value_type>()
			)
			)>
	> : std::true_type {
	};

	template<class T>
	constexpr bool has_emplace_after_v = has_emplace_after<T>::value;

	template<class T, class R = void>
	using enable_if_has_emplace_after = std::enable_if_t<has_emplace_after_v<T>, R>;

	/* is map */
	template<typename T, typename = void>
	struct is_associative_map : std::false_type {};

	template<typename T>
	struct is_associative_map<T,
		std::void_t<typename T::key_type,
		typename T::mapped_type>>
		: std::true_type {};

	template<typename T>
	constexpr bool is_associative_map_v = is_associative_map<T>::value;

	template<typename T, typename R = void>
	using enable_if_associative_map = std::enable_if_t<is_associative_map_v<T>, R>;


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

	/* Serializer function detection */
	template <typename T>
	using serialize_fn = decltype(SerializeImpl(std::declval<const T&>()));

	template<typename T>
	using has_serialize = std::integral_constant<bool, is_detected<serialize_fn, T>::value>;

	template <typename T, typename R>
	using enable_if_has_serialize = std::enable_if_t<has_serialize<T>::value, R>;

	/* Deserializer function detection */
	template <typename T>
	using deserialize_fn = decltype(DeserializeImpl(std::declval<const json&>(), std::declval<T&>()));

	template<typename T>
	using has_deserialize = std::integral_constant<bool, is_detected<deserialize_fn, T>::value>;

	template <typename T, typename R>
	using enable_if_has_deserialize = std::enable_if_t<has_deserialize<T>::value, R>;
}