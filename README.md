# Prefabs
 
My goal is to make an expandable serialization library that allows for prefabs.

Using:
- [nlohmann/json](https://www.github.com/nlohmann/json) for JSON serialization
- [cbeck88/visit_struct](https://github.com/cbeck88/visit_struct) for object reflection
- [cubicdaiya/dtl](https://github.com/cubicdaiya/dtl) for diffing

# Supported types

The following types are supported by default inside the ``<svh/serializer.hpp>`` class.

- Visitable Structs - see [Visit Struct](#visit-struct)
- User-defined types - see [Custom Types](#custom-types)
- Default types
  - ``bool``, ``char``, ``signed char``, ``unsigned char``, ``wchar_t``, ``char16_t``, ``char32_t``, ``short``, ``unsigned short``, ``int``, ``unsigned int``, ``long``, ``unsigned long``, ``long long``, ``unsigned long long``, ``float``, ``double``, ``long double``
- C-style arrays
  - ``bool[]``, ``char[]``, ``int[]`` etc.
- Const char pointer
  - ``const char*``

To include the STL types you can include the ``<svh/std_types.hpp>`` header. This will include all STL types:

- String
  - ``std::string``
- Containers
  - ``std::vector``, ``std::map``, ``std::unordered_map``, ``std::set``, ``std::unordered_set``, ``std::list``, ``std::deque``, ``std::array``, ``std::pair``, ``std::tuple``
- Smart pointers
  - ``std::shared_ptr``, ``std::unique_ptr``, ``std::weak_ptr``
- Variants
  - ``std::variant`` 
- Optional (C++17)
  - ``std::optional``



# Custom Types

## Serialization

There are 2 ways of making custom types serializable/deserializable.
1. Using the `VISITABLE_STRUCT` macro
2. Implementing the `SerializeImpl` function

### Visit Struct

Use the `VISITABLE_STRUCT`, offered by ``<visit_struct/visit_struct.hpp>``, macro to make a struct serializable. This will automatically generate the serialization code for you.

```cpp
struct MyStruct {
	int a = 0;
	float b = 0.0f;
	std::string c = "default";
};

VISITABLE_STRUCT(MyStruct, a, b, c);
```

### Custom Serialize Implementation

You can also implement the `SerializeImpl` and ``DeserializeImpl`` functions for your custom type yourself. **These functions need to be defined inside a header**

```cpp
struct MyStruct {
	int a = 0;
	float b = 0.0f;
	std::string c = "default";
};

static inline svh::json SerializeImpl(const MyStruct& s) {
	svh::json result = svh::json::object();
	result["a"] = svh::Serializer::Serialize(s.a); // int
	result["b"] = svh::Serializer::Serialize(s.b); // float
	result["c"] = svh::Serializer::Serialize(s.c); // string
	return result;
}

static inline void DeserializeImpl(const svh::json& j, MyStruct& s) {
	if (j.is_object()) {
		svh::Deserializer::FromJson(j["a"], s.a); // int
		svh::Deserializer::FromJson(j["b"], s.b); // float
		svh::Deserializer::FromJson(j["c"], s.c); // string
	}
}

```

For external types you need to define them in their respective namespace.

```cpp
#include <glm/glm.hpp>

namespace glm {
	static inline svh::json SerializeImpl(const glm::vec3& v) {
		return svh::json::array({ v.x, v.y, v.z });
	}

	static inline svh::json SerializeImpl(const glm::vec4& v) {
		return svh::json::array({ v.x, v.y, v.z, v.w });
	}
}
```

## Compare

The library needs to be able to "calculate" the difference between 2 objects. Most STL types are supported. But for custom types you need to implement the `CompareImpl` function yourself. For example:

```cpp
struct MyStruct {
	int a = 0;
	float b = 0.0f;
	std::string c = "default";
};

static inline svh::json CompareImpl(const MyStruct& left, const MyStruct& right) {
	svh::json result = svh::json::object();
	if (left.a != right.a) {
		result["a"] = svh::Serializer::ToJson(right.a);
	}
	if (left.b != right.b) {
		result["b"] = svh::Serializer::ToJson(right.b);
	}
	if (left.c != right.c) {
		result["c"] = svh::Serializer::ToJson(right.c);
	}
	return result;
}
```

This function returns a JSON object with the differences between the 2 objects. See [``std:types.hpp``](solution/prefabs/include/svh/std_types.hpp) for examples.

## Overwrite

If the ``CompareImpl`` function does **not** return the same json format as the ``SerializeImpl`` function (so it can not directly be used by ``DeserializeImple``), you need to implement a ``OverwriteImpl`` function.

```cpp
struct MyStruct {
	int a = 0;
	float b = 0.0f;
	std::string c = "default";
};

static inline void OverwriteImpl(const svh::json& j, MyStruct& s) {
	if (j.contains("a")) {
		svh::Deserializer::FromJson(j["a"], s.a);
	}
	if (j.contains("b")) {
		svh::Deserializer::FromJson(j["b"], s.b);
	}
	if (j.contains("c")) {
		svh::Deserializer::FromJson(j["c"], s.c);
	}
}
```

# Tests

The solution also contains a unit test project. These tests are used to test serialization, deserialization, comparing and overwriting.

- [``serialize_test.cpp``](solution/prefabs_tests/serialize_tests.cpp)
- [``deserialize_test.cpp``](solution/prefabs_tests/deserialize_tests.cpp)
- [``compare_test.cpp``](solution/prefabs_tests/compare_tests.cpp) (Also includes overwrite tests)