#pragma once
#include "svh/serializer.hpp"
#include "svh/defines.hpp"

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

/* Serialize Functions */
namespace std {
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
			if constexpr (svh::is_string_v<K>) {
				result[item.first] = svh::Serializer::ToJson(item.second);
			} else {
				auto key = svh::Serializer::ToJson(item.first).dump();
				result[key] = svh::Serializer::ToJson(item.second);
			}
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
			if constexpr (svh::is_string_v<K>) {
				result[item.first] = svh::Serializer::ToJson(item.second);
			} else {
				auto key = svh::Serializer::ToJson(item.first).dump();
				result[key] = svh::Serializer::ToJson(item.second);
			}
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

		for (auto&& [k, v] : mm) {
			auto& slot = result[k];
			if (slot.is_null()) {
				// first value for this key – store it directly
				slot = svh::Serializer::ToJson(v);
			} else if (slot.is_array()) {
				// already collecting duplicates
				slot.push_back(svh::Serializer::ToJson(v));
			} else {
				// second value – convert the existing scalar into an array
				svh::json tmp = std::move(slot);
				slot = svh::json::array();
				slot.push_back(std::move(tmp));
				slot.push_back(svh::Serializer::ToJson(v));
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
		for (auto&& [k, v] : umm) {
			auto& slot = result[k];
			if (slot.is_null()) {
				// first value for this key – store it directly
				slot = svh::Serializer::ToJson(v);
			} else if (slot.is_array()) {
				// already collecting duplicates
				slot.push_back(svh::Serializer::ToJson(v));
			} else {
				// second value – convert the existing scalar into an array
				svh::json tmp = std::move(slot);
				slot = svh::json::array();
				slot.push_back(std::move(tmp));
				slot.push_back(svh::Serializer::ToJson(v));
			}
		}
		return result;
	}

	/* For pairs */
	template<typename T1, typename T2>
	static inline svh::json SerializeImpl(const std::pair<T1, T2>& value) {
		svh::json result = svh::json::array();
		result.push_back(svh::Serializer::ToJson(value.first));
		result.push_back(svh::Serializer::ToJson(value.second));
		return result;
	}

	/* For tuples */
	template<std::size_t N>
	struct TupleSerializer {
		template<typename... Args>
		static void serialize(svh::json& j, const std::tuple<Args...>& t) {
			TupleSerializer<N - 1>::serialize(j, t);
			j.push_back(svh::Serializer::ToJson(std::get<N - 1>(t)));
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

	/* Generic container for vector, array, set, unordered_set, multiset, unordered_multiset, deque, list, foward_list, initializer_lists */
	template<class T>
	static inline auto SerializeImpl(const T& c)
		-> svh::enable_if_has_begin_end<T, svh::json> {
		svh::json result = svh::json::array();
		for (auto const& item : c) {
			result.push_back(svh::Serializer::ToJson(item));
		}
		return result;
	}

#if SVH_HAVE_STD_OPTIONAL
	/* For optionals (only if C++17 and <optional> is available) */
	template<typename T>
	static inline svh::json SerializeImpl(const std::optional<T>& value) {
		if (value) {
			return svh::Serializer::ToJson(*value);
		}
		return nullptr;
	}
#endif

#if SVH_HAVE_STD_VARIANT
	/* For variants (only if C++17 and <variant> is available) */
	template<typename... Ts>
	static inline svh::json SerializeImpl(const std::variant<Ts...>& value) {
		return std::visit([](const auto& v) {
			return svh::Serializer::ToJson(v);
			}
		, value);
	}
#endif


	/* For unique pointers */
	template<typename T, typename Deleter>
	static inline svh::json SerializeImpl(const std::unique_ptr<T, Deleter>& ptr) {
		if (!ptr)
			return nullptr;
		return svh::Serializer::ToJson(*ptr);
	}

	/* For shared pointers */
	template<typename T>
	static inline svh::json SerializeImpl(const std::shared_ptr<T>& ptr) {
		if (!ptr)
			return nullptr;
		return svh::Serializer::ToJson(*ptr);
	}

	/* For weak pointers */
	template<typename T>
	static inline svh::json SerializeImpl(const std::weak_ptr<T>& ptr) {
		if (auto sp = ptr.lock()) {
			return svh::Serializer::ToJson(*sp);
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


/* Deserialize functions */
namespace std {

	/* For maps */
	template<
		typename K, // Key type
		typename V, // Value type
		typename C, // Key comparison type
		typename A  // Allocator type
	>
	static inline void DeserializeImpl(const svh::json& j, std::map<K, V, C, A>& value) {
		if (!j.is_object()) {
			return svh::Deserializer::HandleError("object", j);
		}
		for (const auto& item : j.items()) {
			auto& key = item.key();
			auto& val = item.value();
			K k{};
			if constexpr (svh::is_string_v<K>) {
				k = key;
			} else {
				svh::Deserializer::FromJson(key, k);
			}
			if (val.is_array()) {
				for (const auto& v : val) {
					V temp_value{};
					svh::Deserializer::FromJson(v, temp_value);
					value.emplace(std::move(k), std::move(temp_value));
				}
			} else {
				V temp_value{};
				svh::Deserializer::FromJson(val, temp_value);
				value.emplace(std::move(k), std::move(temp_value));
			}
		}
	}

	/* For unordered maps */
	template<
		typename K, // Key type
		typename V, // Value type
		typename H, // Hash function type
		typename E, // Key equality function type
		typename A  // Allocator type
	>
	static inline void DeserializeImpl(const svh::json& j, std::unordered_map<K, V, H, E, A>& value) {
		if (!j.is_object()) {
			return svh::Deserializer::HandleError("object", j);
		}
		for (const auto& item : j.items()) {
			auto& key = item.key();
			auto& val = item.value();
			K k{};
			svh::Deserializer::FromJson(key, k);
			if (val.is_array()) {
				for (const auto& v : val) {
					V temp_value{};
					svh::Deserializer::FromJson(v, temp_value);
					value.emplace(std::move(k), std::move(temp_value));
				}
			} else {
				V temp_value{};
				svh::Deserializer::FromJson(val, temp_value);
				value.emplace(std::move(k), std::move(temp_value));
			}
		}
	}

	/* For multimap */
	template<
		typename K, // Key type
		typename V, // Value type
		typename C, // Key comparison type
		typename A  // Allocator type
	>
	static inline void DeserializeImpl(const svh::json& j, std::multimap<K, V, C, A>& mm) {
		if (!j.is_object()) {
			return svh::Deserializer::HandleError("object", j);
		}
		for (const auto& item : j.items()) {
			auto& key = item.key();
			auto& val = item.value();
			K k{};
			svh::Deserializer::FromJson(key, k);
			if (val.is_array()) {
				for (const auto& v : val) {
					V value{};
					svh::Deserializer::FromJson(v, value);
					mm.emplace(k, std::move(value));
				}
			} else {
				V value{};
				svh::Deserializer::FromJson(val, value);
				mm.emplace(std::move(k), std::move(value));
			}
		}
	}

	/* For unordered multimaps */
	template<
		typename K, // Key type
		typename V, // Value type
		typename H, // Hash function type
		typename E, // Key equality function type
		typename A  // Allocator type
	>
	static inline void DeserializeImpl(const svh::json& j, std::unordered_multimap<K, V, H, E, A>& umm) {
		if (!j.is_object()) {
			return svh::Deserializer::HandleError("object", j);
		}
		for (const auto& item : j.items()) {
			auto& key = item.key();
			auto& val = item.value();
			K k{};
			svh::Deserializer::FromJson(key, k);
			if (val.is_array()) {
				for (const auto& v : val) {
					V value{};
					svh::Deserializer::FromJson(v, value);
					umm.emplace(k, std::move(value));
				}
			} else {
				V value{};
				svh::Deserializer::FromJson(val, value);
				umm.emplace(std::move(k), std::move(value));
			}
		}
	}

	/* For vectors, lists, dequeus */
	template<class T>
	static inline auto DeserializeImpl(const svh::json& j, T& c)
		-> svh::enable_if_has_emplace_back<T, void> {
		if (!j.is_array()) {
			return svh::Deserializer::HandleError("array", j);
		}
		c.clear(); // Clear the container before deserializing
		for (const auto& item : j) {
			/* Special case for bools since vector<bool> is a proxy-reference container */
			if constexpr (std::is_same_v<typename T::value_type, bool>) {
				bool b{};
				svh::Deserializer::FromJson(item, b);
				c.emplace_back(b);
			} else {
				c.emplace_back();
				auto& slot = c.back();
				svh::Deserializer::FromJson(item, slot);
			}
		}
	}

	/* For arrays */
	template<class E, std::size_t N>
	static inline void DeserializeImpl(const svh::json& j, std::array<E, N>& arr) {
		if (!j.is_array()) { svh::Deserializer::HandleError("array", j); return; }
		if (j.size() != N) { svh::Deserializer::HandleError("array size", j); return; }
		for (std::size_t i = 0; i < N; ++i)
			svh::Deserializer::FromJson(j[i], arr[i]);
	}

	/* set, unordered_set, multiset, unordered_multiset */
	template<class T>
	static inline auto DeserializeImpl(const svh::json& j, T& c)
		-> std::enable_if_t<svh::has_insert_v<T> && !svh::has_emplace_back_v<T>, void> {
		if (!j.is_array()) {
			return svh::Deserializer::HandleError("array", j);
		}

		for (const auto& item : j) {
			auto tmp = typename T::value_type{};
			svh::Deserializer::FromJson(item, tmp);
			c.insert(c.end(), std::move(tmp));
		}
	}

	/* forward_list */
	template<class T>
	static inline auto DeserializeImpl(const svh::json& j, T& c)
		-> std::enable_if_t<svh::has_emplace_after_v<T> && !svh::has_insert_v<T>, void> {
		if (!j.is_array()) {
			return svh::Deserializer::HandleError("array", j);
		}

		auto it = c.before_begin();
		for (auto const& item : j) {
			auto tmp = typename T::value_type{};
			svh::Deserializer::FromJson(item, tmp);
			it = c.emplace_after(it, std::move(tmp));
		}
	}

	/* For tuples */
	template<std::size_t N>
	struct TupleDeserializer {
		template<typename... Args>
		static void deserialize(const svh::json& j, std::tuple<Args...>& t) {
			TupleDeserializer<N - 1>::deserialize(j, t);
			svh::Deserializer::FromJson(j[N - 1], std::get<N - 1>(t));
		}
	};

	// base case: N==0 does nothing
	template<>
	struct TupleDeserializer<0> {
		template<typename... Args>
		static void deserialize(const svh::json&, std::tuple<Args...>&) {}
	};

	template<typename... Args>
	static inline void DeserializeImpl(const svh::json& j, std::tuple<Args...>& value) {
		if (!j.is_array()) {
			return svh::Deserializer::HandleError("array", j);
		}
		TupleDeserializer<sizeof...(Args)>::deserialize(j, value);
	}

	/* For pairs */
	template<typename T1, typename T2>
	static inline void DeserializeImpl(const svh::json& j, std::pair<T1, T2>& value) {
		if (!j.is_array()) {
			return svh::Deserializer::HandleError("array", j);
		}
		if (j.size() != 2) {
			return svh::Deserializer::HandleError("pair size", j);
		}
		svh::Deserializer::FromJson(j[0], value.first);
		svh::Deserializer::FromJson(j[1], value.second);
	}

#if SVH_HAVE_STD_OPTIONAL
	/* For optionals (only if C++17 and <optional> is available) */
	template<typename T>
	static inline void DeserializeImpl(const svh::json& j, std::optional<T>& value) {
		if (j.is_null()) {
			value.reset();
		} else {
			value = T{};
			svh::Deserializer::FromJson(j, *value);
		}
	}
#endif

#if SVH_HAVE_STD_VARIANT

	// Try to deserialize j into a temporary T, and if successful assign into the variant.
	template<typename Variant, typename T>
	bool TryVariantAlternative(const svh::json& j, Variant& var) {
		try {
			T tmp{};
			svh::Deserializer::FromJson(j, tmp);
			var = std::move(tmp);
			return true;
		} catch (...) {
			return false;
		}
	}

	/* For variants (only if C++17 and <variant> is available) */
	template<typename... Ts>
	static inline void DeserializeImpl(const svh::json& j, std::variant<Ts...>& value) {
		bool success = false;

		// Fold over all Ts: stop at first one that works
		(void)std::initializer_list<int>{
			(success || (success = TryVariantAlternative<std::variant<Ts...>, Ts>(j, value)), 0)...
		};

		if (!success) {
			svh::Deserializer::HandleError("variant", j);
		}
	}
#endif

	/* For unique pointers */
	template<typename T, typename Deleter>
	static inline void DeserializeImpl(const svh::json& j, std::unique_ptr<T, Deleter>& ptr) {
		if (j.is_null()) {
			ptr.reset();
		} else {
			ptr = std::make_unique<T>();
			svh::Deserializer::FromJson(j, *ptr);
		}
	}

	/* For shared pointers */
	template<typename T>
	static inline void DeserializeImpl(const svh::json& j, std::shared_ptr<T>& ptr) {
		if (j.is_null()) {
			ptr.reset();
		} else {
			ptr = std::make_shared<T>();
			svh::Deserializer::FromJson(j, *ptr);
		}
	}

	/* For weak pointers */
	template<typename T>
	static inline void DeserializeImpl(const svh::json& j, std::weak_ptr<T>& ptr) {
		if (j.is_null()) {
			ptr.reset();
			return;
		}

		// Try to lock the weak_ptr to get the caller’s shared_ptr
		if (auto sp_existing = ptr.lock()) {
			svh::Deserializer::FromJson(j, *sp_existing);
		} else {
			svh::Deserializer::HandleError("weak_ptr is expired", j);
		}
	}

	/* For shared pointers (with custom deleter) */
	template<typename T, typename Deleter>
	static inline void DeserializeImpl(const svh::json& j, std::shared_ptr<T>& ptr) {
		if (j.is_null()) {
			ptr.reset();
		} else {
			ptr = std::shared_ptr<T>(new T(), Deleter());
			svh::Deserializer::FromJson(j, *ptr);
		}
	}

	/* For weak pointers (with custom deleter) */
	template<typename T, typename Deleter>
	static inline void DeserializeImpl(const svh::json& j, std::weak_ptr<T>& ptr) {
		if (j.is_null()) {
			ptr.reset();
		} else {
			auto sp = std::shared_ptr<T>(new T(), Deleter());
			svh::Deserializer::FromJson(j, *sp);
			ptr = sp;
		}
	}

	/* For strings */
	static inline void DeserializeImpl(const svh::json& j, std::string& value) {
		if (j.is_string()) {
			value = j.get<std::string>();
		} else {
			svh::Deserializer::HandleError("string", j);
		}
	}

	/* For string views */
	static inline void DeserializeImpl(const svh::json& j, std::string_view& value) {
		if (j.is_string()) {
			value = j.get<std::string_view>();
		} else {
			svh::Deserializer::HandleError("string_view", j);
		}
	}
}