#pragma once
#include <svh/nlohmann/json.hpp>
#include <svh/visit_struct/visit_struct.hpp>
#include <svh/dtl/dtl.hpp>
#include <iostream>

#include "defines.hpp"

/* Define SVH_DISABLE_EXCEPTION_HANDLING to disable exceptions */
/* Define SVH_DISABLE_ERROR_LOGGING to disable logging */

//#define SVH_DISABLE_EXCEPTION_HANDLING
//#define SVH_DISABLE_ERROR_LOGGING

namespace svh {

	template<typename T>
	auto UserDefinedSerializeImpl(const T& v)
		-> decltype(SerializeImpl(v)) {
		return SerializeImpl(v);
	}

	class Serializer {
	public: /* API */

		/* For users */
		template<typename T>
		static auto ToJson(const T& value) {
			return SerializeImpl(value);
		}

		/* For visitable struct */
		template<typename T>
		void operator()(const char* name, const T& value) {
			result[name] = SerializeImpl(value);
		}

	private: /* Variables */
		json result;

	private: /* Functions */
		Serializer() : result(json()) {}

		/* For visitable structs only */
		template<typename T>
		static auto SerializeImpl(const T& value)
			-> enable_if_visitable<T, json> {
			Serializer serializer;
			visit_struct::for_each(value, serializer);
			return serializer.result;
		}

		/* For user-defined serialize functions */
		template<typename T>
		static auto SerializeImpl(const T& value)
			-> enable_if_has_serialize<T, json> {
			return UserDefinedSerializeImpl(value);
		}

		/* For numbers */
		template<typename T>
		static auto SerializeImpl(const T& value)
			-> enable_if_number<T, json> {
			return value;
		}

		/* For C-style arrays */
		template<typename T, std::size_t N>
		static auto SerializeImpl(const T(&value)[N]) {
			json result = json::array();
			for (const auto& item : value) {
				result.push_back(SerializeImpl(item));
			}
			return result;
		}

		/* For C-style strings */
		static json SerializeImpl(const char* value) {
			return json(value);
		}
	};

	template<typename T>
	auto UserDefinedDeserializeImpl(const json& v, T& value)
		-> decltype(DeserializeImpl(v, value)) {
		DeserializeImpl(v, value);
	}

	class Deserializer {
	public:
		/* For users */
		template<typename T>
		static void FromJson(const json& j, T& value) {
			DeserializeImpl(j, value);
		}

		/* For visitable struct */
		template<typename T>
		void operator()(const char* name, T& value) {
			auto it = input.find(name);
			if (it != input.end()) {
				DeserializeImpl(it.value(), value);
			} else {
				HandleError(name, input);
			}
		}

		static void HandleError(const char* message, const json& j) {
			std::string error_message = "Error deserializing key: \"";
			error_message += message;
			error_message += "\" from object: ";
			error_message += j.dump();

			// Log the error message
#ifndef SVH_DISABLE_ERROR_LOGGING
			std::cerr << error_message << std::endl;
#endif
#ifndef SVH_DISABLE_EXCEPTION_HANDLING
			throw std::runtime_error(error_message);
#endif
		}
	private:
		/* Variables */
		json input;

	private:
		Deserializer() : input(json()) {}
		/* For visitable structs only */
		template<typename T>
		static auto DeserializeImpl(const json& j, T& value)
			-> enable_if_visitable<T, void> {
			Deserializer deserializer;
			deserializer.input = j;
			visit_struct::for_each(value, deserializer);
		}

		/* For user-defined deserialize functions */
		template<typename T>
		static auto DeserializeImpl(const json& j, T& value)
			-> enable_if_has_deserialize<T, void> {
			return UserDefinedDeserializeImpl(j, value); /* Most overloads are void so retrns nothing */
		}

		/* For numbers */
		template<typename T>
		static auto DeserializeImpl(const json& j, T& value)
			-> enable_if_number<T, void> {
			if (j.is_string()) {
				std::string str = j.get<std::string>();
				if constexpr (std::is_same_v<T, bool>) {
					value = (str == "true");
				} else if constexpr (std::is_floating_point_v<T>) {
					value = static_cast<T>(std::stod(str));
				} else if constexpr (std::is_integral_v<T>) {
					value = static_cast<T>(std::stoll(str));
				} else {
					HandleError("Invalid number type", j);
				}
			} else {
				value = j.get<T>();
			}
		}

		/* For C-style arrays */
		template<typename T, std::size_t N>
		static auto DeserializeImpl(const json& j, T(&value)[N]) {
			for (std::size_t i = 0; i < N; ++i) {
				if (i < j.size()) {
					DeserializeImpl(j[i], value[i]);
				} else {
					HandleError("Array index out of bounds", j);
				}
			}
		}
	};

	constexpr char removed_indices[] = "removed indices";
	constexpr char added_values[] = "added";
	constexpr char index[] = "index";
	constexpr char value[] = "value";

	template<typename T>
	auto UserDefinedCompareImpl(const T& left, const T& right)
		-> decltype(CompareImpl(left, right)) {
		return CompareImpl(left, right);
	}

	class Compare {
	public:

		/* For users */
		/* (ORDER MATTERS) It checks any changes that happend to the right object.*/
		/* So if left.a != right.a, it returns the serialization of right with a */
		template<typename T>
		static json GetChanges(const T& left, const T& right) {
			return GetChangesImpl(left, right);
		}

		/* For visitable struct */
		template<typename T>
		void operator()(const char* name, const T& left, const T& right) {
			auto changes = GetChangesImpl(left, right);
			if (!changes.empty()) {
				result[name] = changes;
			}
		}

	private: /* Variables */
		json result;

	private: /* Functions */
		Compare() : result(json()) {}

		/* For visitable structs only */
		template<typename T>
		static auto GetChangesImpl(const T& left, const T& right)
			-> enable_if_visitable<T, json> {
			Compare compare;
			visit_struct::for_each(left, right, compare);
			return compare.result;
		}

		/* For user-defined compare functions */
		template<typename T>
		static auto GetChangesImpl(const T& left, const T& right)
			-> enable_if_has_compare<T, json> {
			return UserDefinedCompareImpl(left, right);
		}

		// T supports random-access iterators
		template<typename Sequence>
		static auto GetChangesImpl(
			const Sequence& left,
			const Sequence& right
		) -> std::enable_if_t<
			!is_visitable_v<Sequence> &&
			!has_compare_v<Sequence>&&
			has_begin_end_v<Sequence> &&
			!is_string_type_v<Sequence>,
			json
		> {
			using Elem = typename Sequence::value_type;

			/* Run dtl to get changes */
			dtl::Diff<Elem, Sequence> d(left, right);
			d.compose();
			auto ses = d.getSes();
			if (!ses.isChange())
				return {};

			// 2) collect ops into something we can scan for delete+add pairs
			struct Op {
				int type;
				long long beforeIdx, afterIdx;
				Elem value;
			};
			std::vector<Op> ops;
			for (auto const& kv : ses.getSequence()) {
				Op o{ kv.second.type,
					  kv.second.beforeIdx - 1,
					  kv.second.afterIdx - 1,
					  kv.first };
				ops.push_back(o);
			}

			// 3) walk the ops, looking for (delete at i) immediately followed by (add at same i)
			//    treat that as an inner “change” and recurse
			json removed_json = json::array();
			json added_json = json::array();

			for (size_t k = 0; k < ops.size(); ++k) {
				auto& o = ops[k];

				// Only for nested-sequence elements, treat a delete+add at the same index
				// as an “inner” replace and recurse one level deeper
				if constexpr (has_begin_end_v<Elem> && !is_string_type_v<Elem>) {
					if (o.type == dtl::SES_DELETE
						&& k + 1 < ops.size()
						&& ops[k + 1].type == dtl::SES_ADD
						&& ops[k + 1].afterIdx == o.beforeIdx) {
						auto outer = o.beforeIdx;
						Elem oldInner = left[outer];
						Elem newInner = right[outer];

						// recurse into the two inner sequences
						json innerDiff = GetChangesImpl(oldInner, newInner);
						if (!innerDiff.empty()) {
							// flatten removed indices
							if (innerDiff.contains(removed_indices)) {
								for (auto& innerIdxJSON : innerDiff[removed_indices]) {
									json path = json::array();
									path.push_back(outer);
									if (innerIdxJSON.is_array()) {
										// splice all inner elements
										for (auto& e : innerIdxJSON)
											path.push_back(e);
									} else {
										// single integer
										path.push_back(innerIdxJSON);
									}
									removed_json.push_back(std::move(path));
								}
							}

							// flatten added values
							if (innerDiff.contains(added_values)) {
								for (auto& item : innerDiff[added_values]) {
									// item[index] might be an int or array
									auto idxJSON = item[index];
									json path = json::array();
									path.push_back(outer);
									if (idxJSON.is_array()) {
										for (auto& e : idxJSON)
											path.push_back(e);
									} else {
										path.push_back(idxJSON);
									}

									// now build your added entry with the flattened path
									json entry = {
										{ index, std::move(path) },
										{ value, item[value] }
									};
									added_json.push_back(std::move(entry));
								}
							}
						}

						++k;       // skip the paired ADD
						continue;  // move to the next op
					}
				}

				// For all other cases (including atomic types), emit flat deletes/adds:
				if (o.type == dtl::SES_DELETE) {
					// whole-element deletion
					removed_json.push_back(o.beforeIdx);
				} else if (o.type == dtl::SES_ADD) {
					// whole-element insertion
					added_json.push_back(json::object({
						{ index, o.afterIdx },
						{ value, Serializer::ToJson(o.value) }
						}));
				}
				// ignore dtl::SES_COMMON
			}


			// 4) assemble final result
			json result = json::object();
			if (!removed_json.empty()) result[removed_indices] = std::move(removed_json);
			if (!added_json.empty())   result[added_values] = std::move(added_json);
			return result;
		}

		// “For anything else” only enabled if
		// - T is not visitable,
		// - T does not have a user compare,
		// - AND either T has no begin/end OR it is a string.
		template<typename T>
		static auto GetChangesImpl(const T& left, const T& right)
			-> std::enable_if_t<
			!is_visitable_v<T> &&
			!has_compare_v<T> &&
			(
				!has_begin_end_v<T> || // no begin/end
				is_string_type_v<T> // or it’s a string
				),
			json> {
			if (left != right) {
				return Serializer::ToJson(right);
			}
			return {};
		}

	};
}