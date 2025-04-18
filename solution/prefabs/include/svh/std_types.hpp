#pragma once
#include "svh/serializer.hpp"

#include <vector>			// for std::vector
#include <map>				// for std::map
#include <unordered_map>	// for std::unordered_map
#include <string>			// for std::string
#include <memory>			// for std::shared_ptr, std::unique_ptr
#include <set>				// for std::set
#include <unordered_set>	// for std::unordered_set
#include <array>			// for std::array
#include <deque>			// for std::deque
#include <list>				// for std::list
#include <tuple>			// for std::tuple
#include <utility>			// for std::pair

// 1) Optional header found?
#if defined(__has_include)
#if __has_include(<optional>)
#include <optional>
#define SVH_HAVE_STD_OPTIONAL 1
#endif
#if __has_include(<variant>)
#include <variant>
#define SVH_HAVE_STD_VARIANT 1
#endif
#endif


namespace std {
	/* For vectors */
	template<typename T, typename A>
	static inline svh::json SerializeImpl(const std::vector<T, A>& value) {
		svh::json result = svh::json::array();
		for (const auto& item : value) {
			result.push_back(svh::Serializer::Serialize(item));
		}
		return result;
	}

	/* For maps */
	template<
		typename K, // Key type
		typename V, // Value type
		typename C, // Key comparison type
		typename A  // Allocator type
	>
	static inline svh::json SerializeImpl(const std::map<K, V, C, A>& value) {
		svh::json result = svh::json::object();
		for (const auto& item : value) {
			result[item.first] = svh::Serializer::Serialize(item.second);
		}
		return result;
	}

	/* For unordered maps */
	template<
		typename K, // Key type
		typename V, // Value type
		typename H, // Hash function type
		typename E, // Key equality function type
		typename A  // Allocator type
	>
	static inline svh::json SerializeImpl(const std::unordered_map<K, V, H, E, A>& value) {
		svh::json result = svh::json::object();
		for (const auto& item : value) {
			result[item.first] = svh::Serializer::Serialize(item.second);
		}
		return result;
	}

	/* For multimap */
	template<
		typename K, // Key type
		typename V, // Value type
		typename C, // Key comparison type
		typename A  // Allocator type
	>
	static inline svh::json SerializeImpl(const std::multimap<K, V, C, A>& mm) {
		svh::json result = svh::json::object();

		for (auto& item : mm) {
			result[item.first].push_back(svh::Serializer::Serialize(item.second));
		}
		// If the value is a single element, convert it to a single value
		for (auto& [key, value] : result.items()) {
			if (value.is_array() && value.size() == 1) {
				value = std::move(value.front());
			}
		}

		return result;
	}

	/* For unordered multimaps */
	template<
		typename K, // Key type
		typename V, // Value type
		typename H, // Hash function type
		typename E, // Key equality function type
		typename A  // Allocator type
	>
	static inline svh::json SerializeImpl(const std::unordered_multimap<K, V, H, E, A>& umm) {
		svh::json result = svh::json::object();
		for (auto& item : umm) {
			result[item.first].push_back(svh::Serializer::Serialize(item.second));
		}
		// If the value is a single element, convert it to a single value
		for (auto& [key, value] : result.items()) {
			if (value.is_array() && value.size() == 1) {
				value = std::move(value.front());
			}
		}
		return result;
	}

	/* For pairs */
	template<typename T1, typename T2>
	static inline svh::json SerializeImpl(const std::pair<T1, T2>& value) {
		svh::json result = svh::json::array();
		result.push_back(svh::Serializer::Serialize(value.first));
		result.push_back(svh::Serializer::Serialize(value.second));
		return result;
	}

	/* For tuples */
	template<std::size_t N>
	struct TupleSerializer {
		template<typename... Args>
		static void serialize(svh::json& j, const std::tuple<Args...>& t) {
			TupleSerializer<N - 1>::serialize(j, t);
			j.push_back(svh::Serializer::Serialize(std::get<N - 1>(t)));
		}
	};

	// base case: N==0 does nothing
	template<>
	struct TupleSerializer<0> {
		template<typename... Args>
		static void serialize(svh::json&, const std::tuple<Args...>&) {}
	};

	template<typename... Args>
	static inline svh::json SerializeImpl(const std::tuple<Args...>& value) {
		svh::json result = svh::json::array();
		TupleSerializer< sizeof...(Args) >::serialize(result, value);
		return result;
	}

	/* For arrays */
	template<typename T, std::size_t N>
	static inline svh::json SerializeImpl(const std::array<T, N>& value) {
		svh::json result = svh::json::array();
		for (const auto& item : value) {
			result.push_back(svh::Serializer::Serialize(item));
		}
		return result;
	}

	/* For sets */
	template<typename T, typename C, typename A>
	static inline svh::json SerializeImpl(const std::set<T, C, A>& value) {
		svh::json result = svh::json::array();
		for (const auto& item : value) {
			result.push_back(svh::Serializer::Serialize(item));
		}
		return result;
	}

	/* For unordered sets */
	template<typename T, typename H, typename E, typename A>
	static inline svh::json SerializeImpl(const std::unordered_set<T, H, E, A>& value) {
		svh::json result = svh::json::array();
		for (const auto& item : value) {
			result.push_back(svh::Serializer::Serialize(item));
		}
		return result;
	}

	/* For multisets */
	template<typename T, typename C, typename A>
	static inline svh::json SerializeImpl(const std::multiset<T, C, A>& value) {
		svh::json result = svh::json::array();
		for (const auto& item : value) {
			result.push_back(svh::Serializer::Serialize(item));
		}
		return result;
	}
	/* For unordered multisets */
	template<typename T, typename H, typename E, typename A>
	static inline svh::json SerializeImpl(const std::unordered_multiset<T, H, E, A>& value) {
		svh::json result = svh::json::array();
		for (const auto& item : value) {
			result.push_back(svh::Serializer::Serialize(item));
		}
		return result;
	}

	/* For deques */
	template<typename T, typename A>
	static inline svh::json SerializeImpl(const std::deque<T, A>& value) {
		svh::json result = svh::json::array();
		for (const auto& item : value) {
			result.push_back(svh::Serializer::Serialize(item));
		}
		return result;
	}

	/* For lists */
	template<typename T, typename A>
	static inline svh::json SerializeImpl(const std::list<T, A>& value) {
		svh::json result = svh::json::array();
		for (const auto& item : value) {
			result.push_back(svh::Serializer::Serialize(item));
		}
		return result;
	}

	/* For initializer lists */
	template<typename T>
	static inline svh::json SerializeImpl(const std::initializer_list<T>& value) {
		svh::json result = svh::json::array();
		for (const auto& item : value) {
			result.push_back(svh::Serializer::Serialize(item));
		}
		return result;
	}

	/* For forwards lists */
	template<typename T, typename A>
	static inline svh::json SerializeImpl(const std::forward_list<T, A>& value) {
		svh::json result = svh::json::array();
		for (const auto& item : value) {
			result.push_back(svh::Serializer::Serialize(item));
		}
		return result;
	}

#if SVH_HAVE_STD_OPTIONAL
	/* For optionals (only if C++17 and <optional> is available) */
	template<typename T>
	static inline svh::json SerializeImpl(const std::optional<T>& value) {
		if (value) {
			return svh::Serializer::Serialize(*value);
		}
		return nullptr;
	}
#endif

#if SVH_HAVE_STD_VARIANT
	/* For variants (only if C++17 and <variant> is available) */
	template<typename... Ts>
	static inline svh::json SerializeImpl(const std::variant<Ts...>& value) {
		return std::visit([](const auto& v) {
			//
			return svh::Serializer::Serialize(v);
			}, value);
	}
#endif


	/* For unique pointers */
	template<typename T, typename Deleter>
	static inline svh::json SerializeImpl(const std::unique_ptr<T, Deleter>& ptr) {
		if (!ptr)
			return nullptr;
		return svh::Serializer::Serialize(*ptr);
	}

	/* For shared pointers */
	template<typename T>
	static inline svh::json SerializeImpl(const std::shared_ptr<T>& ptr) {
		if (!ptr)
			return nullptr;
		return svh::Serializer::Serialize(*ptr);
	}

	/* For weak pointers */
	template<typename T>
	static inline svh::json SerializeImpl(const std::weak_ptr<T>& ptr) {
		if (auto sp = ptr.lock()) {
			return svh::Serializer::Serialize(*sp);
		}
		return nullptr;
	}

	/* For strings */
	static inline svh::json SerializeImpl(const std::string& value) {
		return value;
	}

	/* For string views */
	static inline svh::json SerializeImpl(const std::string_view& value) {
		return value;
	}
}