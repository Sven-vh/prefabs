#pragma once
#include <svh/nlohmann/json.hpp>
#include <svh/visit_struct/visit_struct.hpp>

#include <type_traits>	// for std::enable_if_t

#include "defines.hpp"

namespace svh {

	/* Still uses nlohmann json but so we can easily swap it for non ordered if wanted */
	using json = nlohmann::ordered_json;

	template<typename T>
	auto UserDefinedImpl(const T& v)
		-> decltype(SerializeImpl(v)) {
		return SerializeImpl(v);
	}

	class Serializer {
	public: /* API */

		/* For users */
		template<typename T>
		static auto Serialize(const T& value) {
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
			return UserDefinedImpl(value);
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

	//ToDo: implement later, should be the same as the serializer
	class Deserializer {
	public:

	private:
	};
}