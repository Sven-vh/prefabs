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

		/* For C-style arrays */
		template<typename T, std::size_t N>
		static auto GetChangesImpl(const T(&left)[N], const T(&right)[N])
			-> std::enable_if_t< !is_visitable_v<T> && !has_compare_v<T>, json> {
			auto l2 = svh::to_std_vector(left);
			auto r2 = svh::to_std_vector(right);

			return GetChangesImpl(l2, r2);
		}

		/* For anything else */
		template<typename T>
		static auto GetChangesImpl(const T& left, const T& right)
			-> std::enable_if_t< !is_visitable_v<T> && !has_compare_v<T> && !is_std_vector_v<T>, json> {
			if (left != right) {
				return Serializer::ToJson(right);
			}
			return {};
		}
	};

	template<typename T>
	auto UserDefinedOverwriteImpl(const json& j, T& value)
		-> decltype(OverwriteImpl(j, value)) {
		OverwriteImpl(j, value);
	}

	class Overwrite {
	public:
		/* For users */
		template<typename T>
		static void FromJson(const json& j, T& value) {
			if (j.is_null()) {
				return;
			}
			OverwriteImpl(j, value);
		}

		/* For visitable struct */
		template<typename T>
		void operator()(const char* name, T& value) {
			auto it = input.find(name);
			if (it != input.end()) {
				OverwriteImpl(it.value(), value);
			}
		}

	private: /* Variables */
		json input;
	private: /* Functions */

		/* For userdefined overwrites*/
		template<typename T>
		static auto OverwriteImpl(const json& j, T& value)
			-> enable_if_has_overwrite<T, void> {
			return UserDefinedOverwriteImpl(j, value);
		}

		/* For visitable structs only */
		template<typename T>
		static auto OverwriteImpl(const json& j, T& value)
			-> enable_if_visitable<T, void> {
			Overwrite overwrite;
			overwrite.input = j;
			visit_struct::for_each(value, overwrite);
		}

		/* For everything else */
		template<typename T>
		static auto OverwriteImpl(const json& j, T& value)
			-> std::enable_if_t < svh::has_overwrite_v<T> == false && !svh::is_visitable_v<T>, void> {
			svh::Deserializer::FromJson(j, value);
		}
	};
}

template<typename T>
inline auto operator==(const T& lhs, const T& rhs)
-> svh::enable_if_visitable<T, bool> {
	return svh::Compare::GetChanges(lhs, rhs).empty();
}

template<typename T>
inline auto operator!=(const T& lhs, const T& rhs)
-> svh::enable_if_visitable<T, bool> {
	return !svh::Compare::GetChanges(lhs, rhs).empty();
}