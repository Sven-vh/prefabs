#pragma once
#include <svh/nlohmann/json.hpp>
#include <svh/visit_struct/visit_struct.hpp>
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
				DeserializeImpl(*it, value);
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
			UserDefinedDeserializeImpl(j, value);
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

		/* For C-style strings */
		static void DeserializeImpl(const json& j, char* value) {
			if (j.is_string()) {
				std::string str = j.get<std::string>();
				//std::strncpy(value, str.c_str(), str.size());
				// strncpy may be unsafe
				if (str.size() >= sizeof(value)) {
					HandleError("C-style string too long", j);
				}
				for (size_t i = 0; i < str.size(); ++i) {
					value[i] = str[i];
				}
				value[str.size()] = '\0'; // Null-terminate the string
			} else {
				HandleError("Invalid C-style string", j);
			}
		}
	};

	class Compare {
	public:

	private:
		json result;
	};
}