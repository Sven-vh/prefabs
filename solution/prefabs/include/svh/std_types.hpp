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

/* Compare functions */
namespace std {

	template<typename Sequence>
	static inline auto CompareImpl(const Sequence& left, const Sequence& right)
		-> std::enable_if_t<svh::is_sequence_v<Sequence> && !svh::is_std_vector_v<Sequence>, svh::json> {
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

		for (size_t k = 0; k < ops.size(); ++k) {
			auto& o = ops[k];

			// — only recurse for sequence‐like Elems (e.g. vector<...>, list<...>, but not int) —
			if constexpr (svh::has_begin_end_v<Elem> && !svh::is_string_type_v<Elem>) {
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

						// flatten any removed indices
						if (innerDiff.contains(svh::REMOVED_INDICES)) {
							for (auto& innerIdx : innerDiff[svh::REMOVED_INDICES]) {
								svh::json path = svh::json::array();
								path.push_back(o.beforeIdx);
								if (innerIdx.is_array()) {
									for (auto& e : innerIdx)
										path.push_back(e);
								} else {
									path.push_back(innerIdx);
								}
								removed_json.push_back(std::move(path));
							}
						}

						// flatten any added values
						if (innerDiff.contains(svh::ADDED_VALUES)) {
							for (auto& item : innerDiff[svh::ADDED_VALUES]) {
								svh::json idx = item[svh::INDEX];
								svh::json path = svh::json::array();
								path.push_back(o.beforeIdx);
								if (idx.is_array()) {
									for (auto& e : idx)
										path.push_back(e);
								} else {
									path.push_back(idx);
								}
								svh::json entry = svh::json::object({
									{ svh::INDEX, std::move(path) },
									{ svh::VALUE, item[svh::VALUE] }
									});
								added_json.push_back(std::move(entry));
							}
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
		if (!removed_json.empty()) result[svh::REMOVED_INDICES] = std::move(removed_json);
		if (!added_json.empty())   result[svh::ADDED_VALUES] = std::move(added_json);
		return result;
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