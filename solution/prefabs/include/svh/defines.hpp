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

	/* has insert */
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

	/* Is String */
	template<typename T>
	constexpr bool is_string_v = std::is_same<T, std::string>::value;

	template<typename T>
	constexpr bool is_string_view_v = std::is_same<T, std::string_view>::value;

	template<typename T>
	constexpr bool is_string_type_v = is_string_v<T> || is_string_view_v<T>;

	/* Is std::pair */
	template<typename T>
	constexpr bool is_std_pair_v = is_specialization<T, std::pair>::value;

	/* Is pointer like */
	template<typename, typename = void>
	struct is_pointer_like : std::false_type {};

	// unique_ptr
	template<typename T, typename D>
	struct is_pointer_like<std::unique_ptr<T, D>> : std::true_type {};

	// shared_ptr
	template<typename T>
	struct is_pointer_like<std::shared_ptr<T>> : std::true_type {};

	template<typename P>
	struct is_pointer_like_impl : is_pointer_like<std::remove_cv_t<P>> {};

	template<typename P>
	constexpr bool is_pointer_like_v = is_pointer_like_impl<P>::value;

	template<typename T, typename R = void>
	using enable_if_pointer_like = std::enable_if_t<is_pointer_like_v<T>, R>;


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
	constexpr bool has_serialize_v = is_detected<serialize_fn, T>::value;

	template <typename T, typename R>
	using enable_if_has_serialize = std::enable_if_t<has_serialize_v<T>, R>;

	/* Deserializer function detection */
	template <typename T>
	using deserialize_fn = decltype(DeserializeImpl(std::declval<const json&>(), std::declval<T&>()));

	template<typename T>
	constexpr bool has_deserialize_v = is_detected<deserialize_fn, T>::value;

	template <typename T, typename R>
	using enable_if_has_deserialize = std::enable_if_t<has_deserialize_v<T>, R>;

	/* Compare function detection */
	template <typename T>
	using compare_fn = decltype(CompareImpl(std::declval<const T&>(), std::declval<const T&>()));

	template<typename T>
	constexpr bool has_compare_v = is_detected<compare_fn, T>::value;

	template <typename T, typename R>
	using enable_if_has_compare = std::enable_if_t<has_compare_v<T>, R>;

	/* For json key names */
	constexpr char REMOVED_INDICES[] = "removed indices";
	constexpr char ADDED_VALUES[] = "added";
	constexpr char INDEX[] = "index";
	constexpr char VALUE[] = "value";
	constexpr char FIRST[] = "first";
	constexpr char SECOND[] = "second";

	/* Is sequence type*/
	template<typename T>
	constexpr bool is_sequence_v = has_begin_end_v<T> && !is_string_type_v<T>;

	/* Is std::vector */
	template<typename T> struct is_std_vector : std::false_type {};
	template<typename U, typename A>
	struct is_std_vector<std::vector<U, A>> : std::true_type {};
	template<typename T>
	inline constexpr bool is_std_vector_v = is_std_vector<
		std::remove_cv_t<std::remove_reference_t<T>>
	>::value;

	/* convert to vector */
	template<typename T>
	auto to_std_vector(const T& x)
		-> std::enable_if_t<!svh::is_sequence_v<T>, T> {
		// atomic – not a container, just return as-is
		return x;
	}

	//decltype(to_std_vector(*std::begin(s))) gets the type of the first element
	template<typename Seq>
	auto to_std_vector(const Seq& s)
		-> std::enable_if_t<svh::is_sequence_v<Seq>, std::vector< decltype(to_std_vector(*std::begin(s)))>> {
		using Inner = decltype(to_std_vector(*std::begin(s)));
		std::vector<Inner> out;
		out.reserve(std::distance(std::begin(s), std::end(s)));
		for (auto const& e : s) {
			out.push_back(to_std_vector(e));
		}
		return out;
	}
}