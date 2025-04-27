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
		svh::json result = svh::json::object();
		auto first = svh::Serializer::ToJson(value.first);
		auto second = svh::Serializer::ToJson(value.second);
		if (first.is_string()) {
			result[first] = second;
		} else {
			result[first.dump()] = second;
		}
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
		if (!j.is_object() || j.contains(svh::ADDED_VALUES) || j.contains(svh::REMOVED) || j.contains(svh::CHANGED_VALUES)) {
			svh::Overwrite::FromJson(j, value); // Use overwrite to handle non-object types
			return;
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
			svh::Overwrite::FromJson(j, value); // Use overwrite to handle non-object types
			return;
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
			svh::Overwrite::FromJson(j, mm); // Use overwrite to handle non-object types
			return;
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
			svh::Overwrite::FromJson(j, umm); // Use overwrite to handle non-object types
			return;
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
			//return svh::Deserializer::HandleError("array", j);
			svh::Overwrite::FromJson(j, c); // Use overwrite to handle non-array types
			return;
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
		if (!j.is_array()) {
			svh::Overwrite::FromJson(j, arr); // Use overwrite to handle non-array types
			return;
		}
		if (j.size() != N) { svh::Deserializer::HandleError("array size", j); return; }
		for (std::size_t i = 0; i < N; ++i)
			svh::Deserializer::FromJson(j[i], arr[i]);
	}

	/* set, unordered_set, multiset, unordered_multiset */
	template<class T>
	static inline auto DeserializeImpl(const svh::json& j, T& c)
		-> std::enable_if_t<svh::has_insert_v<T> && !svh::has_emplace_back_v<T>, void> {
		if (!j.is_array()) {
			svh::Overwrite::FromJson(j, c); // Use overwrite to handle non-array types
			return;
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
			svh::Overwrite::FromJson(j, c); // Use overwrite to handle non-array types
			return;
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
			svh::Overwrite::FromJson(j, value); // Use overwrite to handle non-array types
			return;
		}
		TupleDeserializer<sizeof...(Args)>::deserialize(j, value);
	}

	/* For pairs */
	template<typename T1, typename T2>
	static inline void DeserializeImpl(const svh::json& j, std::pair<T1, T2>& value) {
		auto dump = j.dump();
		if (!j.is_object()) {
			svh::Overwrite::FromJson(j, value); // Use overwrite to handle non-object types
			return;
		}
		auto& key = j.begin().key();
		auto& val = j.begin().value();
		svh::Deserializer::FromJson(key, value.first);
		svh::Deserializer::FromJson(val, value.second);
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

/* Compare functions */
namespace std {

	template<typename Sequence>
	static inline auto CompareImpl(const Sequence& left, const Sequence& right)
		-> std::enable_if_t<svh::is_sequence_v<Sequence> && !svh::is_std_vector_v<Sequence> && !svh::is_associative_map_v<Sequence>, svh::json> {
		// Turn left+right into nested std::vector<…> at all depths:
		auto l2 = svh::to_std_vector(left);
		auto r2 = svh::to_std_vector(right);

		// Now l2/r2 have type std::vector<…vector<…vector<Atomic>…>>
		// and we dispatch into your real CompareImpl(std::vector<Elem>,…).
		return svh::Compare::GetChanges(l2, r2);
	}

	/* For tuples */
	template<typename... Args>
	static inline svh::json CompareImpl(const std::tuple<Args...>& left, const std::tuple<Args...>& right) {
		//convert tuple to std::vector
		auto l1 = svh::to_std_vector(left);
		auto r1 = svh::to_std_vector(right);
		return svh::Compare::GetChanges(l1, r1);
	}

	template<typename T>
	struct CustomCompare {
		bool impl(const T& a, const T& b) const {
			auto changes = svh::Compare::GetChanges(a, b);
			return changes.empty();
		}
	};

	template<typename Elem>
	static inline svh::json CompareImpl(
		const std::vector<Elem>& left,
		const std::vector<Elem>& right
	) {
		// Use the custom comparator to compare elements
		dtl::Diff<Elem, std::vector<Elem>, CustomCompare<Elem>> d(left, right, false, CustomCompare<Elem>{});
		d.compose();
		auto ses = d.getSes();
		if (!ses.isChange()) return {};

		struct Op {
			int type;
			long long beforeIdx, afterIdx;
			Elem value;

			Op(int t, long long b, long long a, Elem v)
				: type(t),
				beforeIdx(b),
				afterIdx(a),
				value(std::move(v)) {
			}
		};

		std::vector<Op> ops;
		for (auto const& kv : ses.getSequence()) {
			ops.emplace_back(
				kv.second.type,
				kv.second.beforeIdx - 1,
				kv.second.afterIdx - 1,
				std::move(kv.first)
			);
		}

		svh::json removed_json = svh::json::array();
		svh::json added_json = svh::json::array();
		svh::json changed_json = svh::json::array();

		for (size_t k = 0; k < ops.size(); ++k) {
			auto& o = ops[k];

			// — only recurse for sequence‐like Elems (e.g. vector<...>, list<...>, but not int) —
			if constexpr (!svh::is_string_type_v<Elem>) {
				if (o.type == dtl::SES_DELETE) {
					// find the matching ADD for this delete, anywhere after it
					auto it = std::find_if(
						ops.begin() + k + 1, ops.end(),
						[&](Op const& x) {
							return x.type == dtl::SES_ADD
								&& x.afterIdx == o.beforeIdx;
						}
					);

					if (it != ops.end()) {
						// recurse into the two inner sequences
						Elem oldInner = left[o.beforeIdx];
						Elem newInner = right[o.beforeIdx];
						svh::json innerDiff = svh::Compare::GetChanges(oldInner, newInner);
						auto dump = innerDiff.dump();

						it->type = dtl::SES_COMMON;

						// always treat any innerDiff as a nested “changed” patch:
						if (!innerDiff.empty()) {
							svh::json idx = svh::json::array({ o.beforeIdx });
							changed_json.push_back(
								svh::json::object({
								{ svh::INDEX, std::move(idx) },
								{ svh::VALUE, std::move(innerDiff) }
									})
							);

						}



						// mark that ADD as “used” so we don’t emit it again
						it->type = dtl::SES_COMMON;
						continue;
					}
				}
			}

			// — fallback: atomic delete or add (always wrap the index as [n]) —
			if (o.type == dtl::SES_DELETE) {
				removed_json.push_back(
					svh::json::array({ o.beforeIdx })
				);
			} else if (o.type == dtl::SES_ADD) {
				added_json.push_back(
					svh::json::object({
						{ svh::INDEX, svh::json::array({ o.afterIdx }) },
						{ svh::VALUE, svh::Serializer::ToJson(o.value) }
						})
				);
			}
		}

		svh::json result = svh::json::object();
		if (!removed_json.empty()) result[svh::REMOVED] = std::move(removed_json);
		if (!added_json.empty())   result[svh::ADDED_VALUES] = std::move(added_json);
		if (!changed_json.empty()) result[svh::CHANGED_VALUES] = std::move(changed_json);
		return result;
	}

	// SFINAE‐guard: only pick this when Map is an associative container
	template<typename Map>
	static inline auto CompareImpl(const Map& left, const Map& right)
		-> svh::enable_if_associative_map<Map, svh::json> {
		using Key = typename Map::key_type;
		using Value = typename Map::mapped_type;

		svh::json added_entries = svh::json::array();
		svh::json removed_keys = svh::json::array();
		svh::json changed = svh::json::array();

		// 1) scan additions in `right`
		for (auto const& item : right) {
			auto const& k = item.first;
			auto const& v = item.second;
			if (left.find(k) == left.end()) {
				added_entries.push_back(svh::Serializer::ToJson(item));
			}
		}


		// 2) scan removals in `left`
		for (auto const& [k, v] : left) {
			if (right.find(k) == right.end()) {
				removed_keys.push_back(svh::Serializer::ToJson(k));
			}
		}

		// 3) scan common keys for internal changes
		for (auto const& [k, v] : left) {
			auto rit = right.find(k);
			if (rit != right.end()) {
				auto cd = svh::Compare::GetChanges(v, rit->second);
				if (!cd.empty()) {
					//changed[svh::Serializer::ToJson(k).get<std::string>()] = cd;
					auto key = svh::Serializer::ToJson(k);
					changed.push_back(
						svh::json::object({ {key,cd} })
					);
				}
			}
		}

		// 3) build the result only with non-empty arrays/objects
		svh::json result = svh::json::object();
		if (!removed_keys.empty()) result[svh::REMOVED] = std::move(removed_keys);
		if (!changed.empty()) result[svh::CHANGED_VALUES] = std::move(changed);
		if (!added_entries.empty()) result[svh::ADDED_VALUES] = std::move(added_entries);
		return result.empty() ? nullptr : result;
	}



	/* For unique pointers */
	template<typename T, typename Deleter>
	static inline svh::json CompareImpl(const std::unique_ptr<T, Deleter>& left, const std::unique_ptr<T, Deleter>& right) {
		if (!left && !right) {
			return {};
		} else if (!left || !right) {
			svh::Deserializer::HandleError("unique_ptr", svh::json());
			return {};
		} else {
			return svh::Compare::GetChanges(*left, *right);
		}
	}

	/* For shared pointers */
	template<typename T>
	static inline svh::json CompareImpl(const std::shared_ptr<T>& left, const std::shared_ptr<T>& right) {
		if (!left && !right) {
			return {};
		} else if (!left || !right) {
			svh::Deserializer::HandleError("shared_ptr", svh::json());
			return {};
		} else {
			return svh::Compare::GetChanges(*left, *right);
		}
	}

	/* For weak pointers */
	template<typename T>
	static inline svh::json CompareImpl(const std::weak_ptr<T>& left, const std::weak_ptr<T>& right) {
		if (auto sp_left = left.lock()) {
			if (auto sp_right = right.lock()) {
				return svh::Compare::GetChanges(*sp_left, *sp_right);
			} else {
				svh::Deserializer::HandleError("weak_ptr", svh::json());
				return {};
			}
		} else {
			svh::Deserializer::HandleError("weak_ptr", svh::json());
			return {};
		}
	}

	/* For shared pointers (with custom deleter) */
	template<typename T, typename Deleter>
	static inline svh::json CompareImpl(const std::shared_ptr<T>& left, const std::shared_ptr<T>& right) {
		if (!left && !right) {
			return {};
		} else if (!left || !right) {
			svh::Deserializer::HandleError("shared_ptr", svh::json());
			return {};
		} else {
			return svh::Compare::GetChanges(*left, *right);
		}
	}

	/* For weak pointers (with custom deleter) */
	template<typename T, typename Deleter>
	static inline svh::json CompareImpl(const std::weak_ptr<T>& left, const std::weak_ptr<T>& right) {
		if (auto sp_left = left.lock()) {
			if (auto sp_right = right.lock()) {
				return svh::Compare::GetChanges(*sp_left, *sp_right);
			} else {
				svh::Deserializer::HandleError("weak_ptr", svh::json());
				return {};
			}
		} else {
			svh::Deserializer::HandleError("weak_ptr", svh::json());
			return {};
		}
	}

	/* For Pair */
	template<typename T1, typename T2>
	static inline svh::json CompareImpl(const std::pair<T1, T2>& left, const std::pair<T1, T2>& right) {
		if (left == right) {
			return {};
		}
		svh::json result = svh::json::object();
		auto first_changes = svh::Compare::GetChanges(left.first, right.first);
		auto second_changes = svh::Compare::GetChanges(left.second, right.second);
		if (!first_changes.empty()) {
			result[svh::FIRST] = first_changes;
		}
		if (!second_changes.empty()) {
			result[svh::SECOND] = second_changes;
		}
		return result;
	}
}

/* Overwrite functions */
namespace std {

	/* --- Overwrite one change, descending through nested tuples if needed --- */
	template<std::size_t N>
	struct TupleOverwriter {
		template<typename... Args>
		static void overwrite(const svh::json& change, std::tuple<Args...>& t) {
			// 1) extract the index
			const auto& idx_arr = change["index"];
			if (!idx_arr.is_array() || idx_arr.empty()) {
				return svh::Deserializer::HandleError(
					"array with at least one index", idx_arr);
			}
			std::size_t idx = idx_arr[0].get<std::size_t>();

			// 2) if it matches this position, apply the change here
			if (idx == N - 1) {
				auto& elem = std::get<N - 1>(t);
				const auto& val = change["value"];

				if constexpr (svh::is_std_tuple_v<std::decay_t<decltype(elem)>>) {
					// element is itself a tuple: if the JSON is a diff obj, recurse,
					// otherwise treat it as a full replace (array) and deserialize
					if (val.is_object() && val.contains("changed") && val["changed"].is_array()) {
						OverwriteImpl(val, elem);
					} else {
						DeserializeImpl(val, elem);
					}
				} else {
					// a leaf element – just overwrite via the normal FromJson
					svh::Deserializer::FromJson(val, elem);
				}
			} else {
				// not the right slot — keep recursing down
				TupleOverwriter<N - 1>::overwrite(change, t);
			}
		}
	};

	template<>
	struct TupleOverwriter<0> {
		template<typename... Args>
		static void overwrite(const svh::json&, std::tuple<Args...>&) {
			// ran off the front, nothing to do
		}
	};

	/* --- The top‐level overwrite entry point --- */
	template<typename... Args>
	static inline void OverwriteImpl(const svh::json& j, std::tuple<Args...>& t) {
		if (!j.is_object()) {
			return svh::Deserializer::HandleError("object", j);
		}
		auto it = j.find("changed");
		if (it == j.end() || !it->is_array()) {
			return;  // no diffs to apply
		}
		for (const auto& change : *it) {
			TupleOverwriter<sizeof...(Args)>::overwrite(change, t);
		}
	}

	static inline std::size_t getIndex(const svh::json& idx) {
		if (idx.is_array()) return idx[0].get<std::size_t>();
		return idx.get<std::size_t>();
	}

	// 3a) generic vector<Elem>
	template<typename Elem>
	static inline void OverwriteImpl(const svh::json& j, std::vector<Elem>& c) {
		if (j.is_array()) {
			c.clear();
			for (auto const& item : j) {
				Elem tmp{};
				svh::Overwrite::FromJson(item, tmp);
				c.emplace_back(std::move(tmp));
			}
			return;
		}
		if (!j.is_object()) {
			svh::Deserializer::HandleError("vector", j);
			return;
		}
		// REMOVED
		if (j.contains(svh::REMOVED)) {
			int offset = 0;
			for (auto const& idx : j[svh::REMOVED]) {
				auto dump = idx.dump();
				auto i = getIndex(idx);
				i -= offset;
				if (i < c.size()) {
					c.erase(c.begin() + i);
					++offset;
				} else {
					svh::Deserializer::HandleError("index out of range", j);
				}
			}
		}
		// ADDED
		if (j.contains(svh::ADDED_VALUES)) {
			for (auto const& item : j[svh::ADDED_VALUES]) {
				std::size_t i = getIndex(item[svh::INDEX]);
				Elem tmp{};
				auto dump = item[svh::VALUE].dump();
				svh::Overwrite::FromJson(item[svh::VALUE], tmp);
				i = std::min(i, c.size());
				c.insert(c.begin() + i, std::move(tmp));
			}
		}
		// CHANGED
		if (j.contains(svh::CHANGED_VALUES)) {
			for (auto const& item : j[svh::CHANGED_VALUES]) {
				std::size_t i = getIndex(item[svh::INDEX]);
				if (i < c.size()) {
					svh::Overwrite::FromJson(item[svh::VALUE], c[i]);
				} else {
					svh::Deserializer::HandleError("index out of range", j);
				}
			}
		}
	}

	// 3b) vector<bool>
	static inline void OverwriteImpl(const svh::json& j, std::vector<bool>& c) {
		if (j.is_array()) {
			c.clear();
			for (auto const& item : j) {
				bool b{};
				svh::Overwrite::FromJson(item, b);
				c.emplace_back(b);
			}
			return;
		}
		if (!j.is_object()) {
			svh::Deserializer::HandleError("vector<bool>", j);
			return;
		}
		if (j.contains(svh::REMOVED)) {
			int offset = 0;
			for (auto const& idx : j[svh::REMOVED]) {
				auto i = getIndex(idx);
				i -= offset;
				if (i < c.size()) {
					c.erase(c.begin() + i);
					++offset;
				} else {
					svh::Deserializer::HandleError("index out of range", j);
				}
			}
		}
		if (j.contains(svh::ADDED_VALUES)) {
			for (auto const& item : j[svh::ADDED_VALUES]) {
				auto i = getIndex(item[svh::INDEX]);
				bool b{};
				svh::Overwrite::FromJson(item[svh::VALUE], b);
				i = std::min(i, c.size());
				c.insert(c.begin() + i, b);
			}
		}
		if (j.contains(svh::CHANGED_VALUES)) {
			for (auto const& item : j[svh::CHANGED_VALUES]) {
				auto i = getIndex(item[svh::INDEX]);
				if (i < c.size()) {
					bool b{};
					svh::Overwrite::FromJson(item[svh::VALUE], b);
					c[i] = b;
				} else {
					svh::Deserializer::HandleError("index out of range", j);
				}
			}
		}
	}

	// 3c) list<Elem>
	template<typename Elem>
	static inline void OverwriteImpl(const svh::json& j, std::list<Elem>& c) {
		auto vec = svh::to_std_vector(c);
		svh::Overwrite::FromJson(j, vec);
		c.clear();
		for (auto const& item : vec) {
			svh::Overwrite::FromJson(item, c.emplace_back());
		}
	}

	// 3c) forward_list<Elem>
	template<typename Elem>
	static inline void OverwriteImpl(const svh::json& j, std::forward_list<Elem>& c) {
		auto vec = svh::to_std_vector(c);
		svh::Overwrite::FromJson(j, vec);

		c.clear();

		auto it = c.before_begin();
		for (auto const& item : vec) {
			it = c.insert_after(it, Elem{});
			svh::Overwrite::FromJson(item, *it);
		}
	}


	// 3d) deque<Elem>
	template<typename Elem>
	static inline void OverwriteImpl(const svh::json& j, std::deque<Elem>& c) {
		auto vec = svh::to_std_vector(c);
		svh::Overwrite::FromJson(j, vec);
		c.clear();
		for (auto const& item : vec) {
			svh::Overwrite::FromJson(item, c.emplace_back());
		}
	}

	/* Std array */
	template<typename Elem, std::size_t N>
	static inline void OverwriteImpl(const svh::json& j, std::array<Elem, N>& arr) {
		auto vec = svh::to_std_vector(arr);
		svh::Overwrite::FromJson(j, vec);
		for (std::size_t i = 0; i < N; ++i) {
			svh::Overwrite::FromJson(vec[i], arr[i]);
		}
	}

	/* C Style Arrays */
	template<typename Elem, std::size_t N>
	static inline void OverwriteImpl(const svh::json& j, Elem(&arr)[N]) {
		auto vec = svh::to_std_vector(arr);
		svh::Overwrite::FromJson(j, vec);
		for (std::size_t i = 0; i < N; ++i) {
			svh::Overwrite::FromJson(vec[i], arr[i]);
		}
	}

	// 4) Associative maps
	template<typename Map>
	static inline auto OverwriteImpl(const svh::json& j, Map& m)
		-> svh::enable_if_associative_map<Map, void> {
		using Key = typename Map::key_type;
		using Value = typename Map::mapped_type;

		// null → no changes
		if (j.is_null()) {
			return;
		}

		// array → full replace
		if (j.is_array()) {
			m.clear();
			for (auto const& item : j) {
				if (!item.is_object()) {
					svh::Deserializer::HandleError("map", item);
					continue;
				}
				// each entry is a singleton object { key: value }
				for (auto it = item.begin(); it != item.end(); ++it) {
					Key   k;
					Value v;
					// parse the key (string) back into Key
					svh::Overwrite::FromJson(svh::json(it.key()), k);
					// parse the value
					svh::Overwrite::FromJson(it.value(), v);
					m.emplace(std::move(k), std::move(v));
				}
			}
			return;
		}

		// must be object from here on
		if (!j.is_object()) {
			svh::Deserializer::HandleError("map", j);
			return;
		}

		// 1) removals
		if (j.contains(svh::REMOVED)) {
			for (auto const& keyJ : j[svh::REMOVED]) {
				Key k;
				svh::Overwrite::FromJson(keyJ, k);
				m.erase(k);
			}
		}

		auto dump = j.dump();

		// 2) additions
		if (j.contains(svh::ADDED_VALUES)) {
			for (auto const& item : j[svh::ADDED_VALUES]) {
				auto dump2 = item.dump();
				if (!item.is_object()) {
					svh::Deserializer::HandleError("map", item);
					continue;
				}
				for (auto it = item.begin(); it != item.end(); ++it) {
					Key   k;
					Value v;
					svh::Overwrite::FromJson(svh::json(it.key()), k);
					svh::Overwrite::FromJson(it.value(), v);
					m.emplace(std::move(k), std::move(v));
				}
			}
		}

		// 3) in‐place changes
		if (j.contains(svh::CHANGED_VALUES)) {
			const auto& changed = j[svh::CHANGED_VALUES];

			// Case A: object of key→value
			if (changed.is_object()) {
				for (auto it = changed.begin(); it != changed.end(); ++it) {
					const auto& keyStr = it.key();
					svh::json keyJ;
					try {
						keyJ = svh::json::parse(keyStr);
					} catch (...) {
						keyJ = keyStr;
					}
					Key k;
					svh::Overwrite::FromJson(keyJ, k);

					auto mapIt = m.find(k);
					if (mapIt != m.end()) {
						svh::Overwrite::FromJson(it.value(), mapIt->second);
					} else {
						svh::Deserializer::HandleError("map", j);
					}
				}

				// Case B: array of single‐pair objects
			} else if (changed.is_array()) {
				for (const auto& entry : changed) {
					if (!entry.is_object()) {
						continue;  // skip anything unexpected
					}
					for (auto it2 = entry.begin(); it2 != entry.end(); ++it2) {
						const auto& keyStr = it2.key();
						svh::json keyJ;
						try {
							keyJ = svh::json::parse(keyStr);
						} catch (...) {
							keyJ = keyStr;
						}
						Key k;
						svh::Overwrite::FromJson(keyJ, k);

						auto mapIt = m.find(k);
						if (mapIt != m.end()) {
							svh::Overwrite::FromJson(it2.value(), mapIt->second);
						} else {
							svh::Deserializer::HandleError("map", j);
						}
					}
				}

			} else {
				// Neither object nor array?  Handle error or ignore.
				svh::Deserializer::HandleError("changed", j);
			}
		}
	}

	// ------------------------------------------------------------------------
	// 5) unique_ptr / shared_ptr / weak_ptr
	// ------------------------------------------------------------------------
	template<typename T, typename Deleter>
	static inline void OverwriteImpl(const svh::json& j, std::unique_ptr<T, Deleter>& p) {
		if (j.is_null()) {
			p.reset();
		} else {
			if (!p) p = std::make_unique<T>();
			svh::Overwrite::FromJson(j, *p);
		}
	}

	template<typename T>
	static inline void OverwriteImpl(const svh::json& j, std::shared_ptr<T>& p) {
		if (j.is_null()) {
			p.reset();
		} else {
			if (!p) p = std::make_shared<T>();
			svh::Overwrite::FromJson(j, *p);
		}
	}

	template<typename T>
	static inline void OverwriteImpl(const svh::json& j, std::weak_ptr<T>& wp) {
		if (auto sp = wp.lock()) {
			svh::Overwrite::FromJson(j, *sp);
		} else {
			svh::Deserializer::HandleError("weak_ptr", j);
		}
	}

	// ------------------------------------------------------------------------
	// 6) pair<T1,T2>
	// ------------------------------------------------------------------------
	template<typename A, typename B>
	static inline void OverwriteImpl(const svh::json& j, std::pair<A, B>& p) {
		if (!j.is_object()) {
			svh::Deserializer::HandleError("pair", j);
			return;
		}
		if (j.contains(svh::FIRST))  svh::Overwrite::FromJson(j[svh::FIRST], p.first);
		if (j.contains(svh::SECOND)) svh::Overwrite::FromJson(j[svh::SECOND], p.second);
	}

	template<
		template<class, class, class> class Set,
		typename Key,
		typename CmpOrHash,
		typename Alloc
	>
	static inline auto OverwriteImpl(
		const svh::json& j,
		Set<Key, CmpOrHash, Alloc>& s
	) -> std::enable_if_t< !svh::is_string_type_v< Set<Key, CmpOrHash, Alloc>>, void> {
		using Container = Set<Key, CmpOrHash, Alloc>;
		// 1) serialize current into a vec repr
		auto vec = svh::to_std_vector(s);

		// 2) overwrite that repr from JSON
		svh::Overwrite::FromJson(j, vec);

		// 3) rebuild the entire container in one go
		Container rebuilt = svh::rebuild_from_vector<Container>(vec);

		// 4) commit it
		s = std::move(rebuilt);
	}

	template<
		template<class, class, class, class> class Set,
		typename Key,
		typename Hash,
		typename KeyEq,
		typename Alloc
	>
	static inline auto OverwriteImpl(
		const svh::json& j,
		Set<Key, Hash, KeyEq, Alloc>& s
	) -> std::enable_if_t<!svh::is_associative_map_v<Set<Key, Hash, KeyEq, Alloc>>, void> {
		using Container = Set<Key, Hash, KeyEq, Alloc>;
		auto dump = j.dump();
		auto vec = svh::to_std_vector(s);
		svh::Overwrite::FromJson(j, vec);
		s = svh::rebuild_from_vector<Container>(vec);
	}


}  // namespace std