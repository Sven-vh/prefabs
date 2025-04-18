# Prefabs
 
My goal is to make an expandable serialization library that allows for prefabs.

Using:
- [nlohmann/json](https://www.github.com/nlohmann/json) for JSON serialization
- [cbeck88/visit_struct](https://github.com/cbeck88/visit_struct) for object reflection

# Types

## Supported types

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



## Custom Types

There are 2 ways of making custom types serializable:

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

You can also implement the `SerializeImpl` function for your custom type. This function should take a reference to your type and return a `svh::json` object.

```cpp
struct MyStruct {
	int a = 0;
	float b = 0.0f;
	std::string c = "default";
};

inline svh::json SerializeImpl(const MyStruct& s) {
	svh::json result = svh::json::object();
	result["a"] = svh::Serializer::Serialize(s.a); // int
	result["b"] = svh::Serializer::Serialize(s.b); // float
	result["c"] = svh::Serializer::Serialize(s.c); // string
	return result;
}
```

### External Types

You can also implement the `SerializeImpl` function for external types. This is useful for types that are not defined in your code, such as types from third-party libraries. These need to be defined their namespace.

```cpp
#include <glm/glm.hpp>

namespace glm {
	inline svh::json SerializeImpl(const glm::vec3& v) {
		return svh::json::array({ v.x, v.y, v.z });
	}

	inline svh::json SerializeImpl(const glm::vec4& v) {
		return svh::json::array({ v.x, v.y, v.z, v.w });
	}
}
```